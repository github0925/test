/************************************************
 * Copyright Semidrive 2020 . All Rights Reserved.
 * **********************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "DDR_regs.h"
#include "ddr_init_helper.h"

#define MAX_ARGV_NUM    8
#define MAX_ACT_NUM     2048

#define IS_HEX_CHAR(x)   \
        (((x >= 'a') && (x <= 'z'))\
            || ((x >= 'A') && (x <= 'Z'))\
            || ((x >= '0') && (x <= '9')))

//#define TEST
#define DBG_LEVEL 0

#define SEQ_HDR_TAG     0xEB901001
#define SEQ_ENTRY_TAG   0xEB914001
#define MAX_SEQ_NUM     8

typedef struct {
    uint32_t tag;       /* EB914001 */
    uint32_t id;        /* match if (board_id|ddr_id) & msk = id */
    uint32_t msk;
    uint32_t start;     /* relative the begining of output file */
    uint32_t size;      /* the size of the seq */
    uint32_t seq_crc32; /* the crc of the seq */
    uint32_t rsvd;
    uint8_t  name[32];  /* to save the name of the seq c file */
    uint32_t crc32;     /* the crc of this data structure */
} seq_entry_t;

typedef struct {
    uint32_t tag;       /* EB901001 */
    uint8_t num;        /* how many seqs in it */
    uint8_t rsvd[7];
    uint32_t crc32;     /* crc32 = cal_crc32(tag | num | rsvd) */
    seq_entry_t entry[0];
} seq_hdr_t;

extern uint32_t cal_crc32(void *buf, size_t size);

static void usage(void)
{
    printf("Usage: ddr_seq_parser if=ddr_seq1.c:ddr_seq2.c:ddr_seq3.c:ddr_seq4.c "
           "id=0x12:0x23:0x34:0x45 msk=0x01:0x2:0x03:0x04 of=ddr_seq.bin gen_hdr"
           "\n\tif: ddr init seq c file, 8 at most"
           "\n\tid: match if (plat_get_id() & msk) = id; 8 at most; shall be in same number as if"
           "\n\tmsk: 0 if not provisioned"
           "\n\tgen_hdr: to generate sequence header even if only one input seq c file\n"
          );
}

static bool is_number(char *s)
{
    bool ret = true;

    if (*s == '0' && (*(s + 1) == 'x') || (*(s + 1) == 'X')) {
        s += 2;

        while (*s != '\0') {
            if (!((*s >= 'A' && *s <= 'F')
                  || (*s >= 'a' && *s <= 'f')
                  || (*s >= '0' && *s <= '9'))) {
                ret = false;
                break;
            }

            s++;
        }
    } else {
        while (*s != '\0') {
            if (!(*s >= '0' && *s <= '9')) {
                ret = false;
                break;
            }

            s++;
        }

    }

    return ret;
}

static int cal_exp(char *exp, uint32_t *val)
{
    char *oprnd[4] = {NULL};
    char *pp = exp;
    char op[3] = {0};
    uint32_t op_cnt = 0;
    uint32_t oprnd_cnt = 1;
    uint32_t v = 0;

    oprnd[0] = exp;

    while (*pp != '\0') {
        if (*pp == '*') {
            op[op_cnt++] = '*';
        } else if (*pp == '+') {
            op[op_cnt++] = '+';
        }

        if ((*pp == '*') || (*pp == '+')) {
            char *p = pp - 1;

            while (*p == ' ') p--;

            *(p + 1) = '\0';
            pp++;

            while (*pp == ' ') pp++;

            oprnd[oprnd_cnt++] = pp;
        }

        pp++;
    }

#if DBG_LEVEL > 1

    for (int i = 0; i < oprnd_cnt; i++) {
        printf("oprnd_%d = %s\n", i, oprnd[i]);
    }

#endif

    if (op_cnt > 0) {
        int j = 0;

        for (; j < op_cnt; j++) {
            if (op[j] == '*') {
                break;
            }
        }

        if (j < op_cnt) {
            uint32_t v1, v2;

            if (!get_val_by_name(oprnd[j], &v1)) {
                if (!is_number(oprnd[j])) {
                    printf("%s is not a number nor valid symbol\n", oprnd[j]);
                    return -1;
                }

                v1 = strtoul(oprnd[j], NULL, 0);
            }

            if (!get_val_by_name(oprnd[j + 1], &v2)) {
                if (!is_number(oprnd[j + 1])) {
                    printf("%s is not a number nor valid symbol\n", oprnd[j + 1]);
                    return -2;
                }

                v2 = strtoul(oprnd[j + 1], NULL, 0);
            }

            v = v1 * v2;
        }

        int k = 0;

        for (; k < op_cnt; k++) {
            if (op[k] == '+') {
                break;
            }
        }

        if (k < op_cnt) {
            uint32_t v1, v2;

            if (!get_val_by_name(oprnd[k], &v1)) {
                if (!is_number(oprnd[k])) {
                    printf("%s is not a number nor valid symbol\n", oprnd[k]);
                    return -3;
                }

                v1 = strtoul(oprnd[k], NULL, 0);
            }

            if (j < op_cnt) {
                v2 = v;
            } else if (!get_val_by_name(oprnd[k + 1], &v2)) {
                if (!is_number(oprnd[k + 1])) {
                    printf("%s is not a number nor valid symbol\n", oprnd[k + 1]);
                    return -4;
                }

                v2 = strtoul(oprnd[k + 1], NULL, 0);
            }

            v = v1 + v2;
        }
    } else {
        if (!get_val_by_name(exp, &v)) {
            if (!is_number(exp)) {
                printf("%s is not a number nor valid symbol\n", exp);
                return -5;
            }

            v = strtoul(exp, NULL, 0);
        }
    }

    *val = v;
    return 0;
}

static int fill_func_act(int argc, char *argv[], int argc_expected,
                         uint32_t func, uint32_t line, ddr_func_call_t *call)
{
    if (argc != argc_expected) {
        printf("Opps, wrong syntax at line %d, %d para expected but %d\n", line, argc_expected - 1,
               argc - 1);
        return -1;
    }

    call->act = ACT_FUNC_CALL;
    call->func = func;

    for (int i = 1; i < argc; i++) {
        if (0 != cal_exp(argv[i], &(call->para[i - 1]))) {
            printf("Opps, wrong paras at line %d\n", line);
            return -1;
        }
    }

#if DBG_LEVEL > 0
    printf("ddr_func_call: act=%x func= 0x%x para[0]=%x para[1]=%d\n",
           call->act, call->func, call->para[0], call->para[1]);
#endif

    return 0;
}

int main(int argc, char *argv[])
{
    char *file_i_name[MAX_SEQ_NUM + 1] = {NULL};
    char *file_o_name = NULL;
    FILE *file_i = NULL;
    FILE *file_o = NULL;
    char *buf = NULL;
    char *seq_buf = NULL;
    int res = 0, seq_num = 0, id_num = 0, msk_num = 0;
    uint32_t id[MAX_SEQ_NUM + 1] = {0};
    uint32_t msk[MAX_SEQ_NUM + 1] = {0};
    seq_hdr_t *seq_hdr = NULL;
    bool gen_hdr = false;

    if (argc < 2) {
        res = -1;
        usage();
        goto cleanup;
    }

    for (unsigned int i = 1; i < argc; i++) {
        if (0 == strncmp(argv[i], "if=", 3)) {
            char *p = argv[i] + 3;
            file_i_name[0] = p;
            seq_num = 1;

            while (*p) {
                if (*p == ':') {
                    if (seq_num > MAX_SEQ_NUM) {
                        printf("Opps, %d input files at most\n", MAX_SEQ_NUM);
                        goto cleanup;
                    }
                    *p = '\0';
                    file_i_name[seq_num++] = p + 1;
                }

                p++;
            }

#if DBG_LEVEL > 1
            printf("if=");

            for (int j = 0; j < seq_num; j++) {
                printf("%s:\n", file_i_name[j]);
            }

            printf("\n");
#endif
        } else if (0 == strncmp(argv[i], "id=", 3)) {
            char *p1 = argv[i] + 3;
            char *p2 = p1;

            while (*p2) {
                if (*p2 == ':') {
                    *p2 = '\0';
                    id[id_num] = strtoul(p1, NULL, 0);
                    p1 = p2 + 1;

                    if (id_num >= MAX_SEQ_NUM - 1) {
                        printf("Opps, %d ids at most\n", MAX_SEQ_NUM);
                        goto cleanup;
                    }
                    id_num++;
                }

                p2++;
            }

            id[id_num++] = strtoul(p1, NULL, 0);
#if DBG_LEVEL > 1
            printf("id=");

            for (int j = 0; j < id_num; j++) {
                printf("0x%x:", id[j]);
            }

            printf("\n");
#endif
        } else if (0 == strncmp(argv[i], "msk=", 4)) {
            char *p1 = argv[i] + 4;
            char *p2 = p1;

            while (*p2) {
                if (*p2 == ':') {
                    *p2 = '\0';
                    msk[msk_num] = strtoul(p1, NULL, 0);
                    p1 = p2 + 1;

                    if (msk_num >= MAX_SEQ_NUM - 1) {
                        printf("Opps, %d msks at most\n", MAX_SEQ_NUM);
                        goto cleanup;
                    }
                    msk_num++;
                }

                p2++;
            }

            msk[msk_num++] = strtoul(p1, NULL, 0);
#if DBG_LEVEL > 1
            printf("msk=");

            for (int j = 0; j < msk_num; j++) {
                printf("0x%x:", msk[j]);
            }

            printf("\n");
#endif
        } else if (0 == strncmp(argv[i], "of=", 3)) {
            file_o_name = argv[i] + 3;
        } else if (0 == strncmp(argv[i], "gen_hdr", 7)) {
            gen_hdr = true;
        }
    }

    if ((file_o_name == NULL) || (seq_num == 0)
        || ((seq_num > 1) && (id_num != seq_num))) {
        res = -2;
        usage();
        goto cleanup;
    }

    file_o = fopen(file_o_name, "wb+");

    if (NULL == file_o) {
        printf("Opps, failed to open %s\n", file_o_name);
        res = -3;
        goto cleanup;
    }

    seq_hdr = (seq_hdr_t *)malloc(sizeof(seq_hdr_t) + sizeof(seq_entry_t) * seq_num);

    if (NULL == seq_hdr) {
        printf("Opps, failed to malloc\n");
        res = -4;
        goto cleanup;
    }

    memset(seq_hdr, 0, sizeof(seq_hdr_t) + sizeof(seq_entry_t) * seq_num);

    if ((seq_num > 1u) || gen_hdr) {
        /* placehodler for hdr/entry, will be re-written finally */
        fwrite(seq_hdr, 1, sizeof(seq_hdr_t) + sizeof(seq_entry_t) * seq_num, file_o);
    }

    seq_hdr->tag = SEQ_HDR_TAG;
    seq_hdr->num = seq_num;
    seq_hdr->crc32 = cal_crc32(seq_hdr, sizeof(seq_hdr_t) - 4);

    uint32_t pos = 0;

    if ((seq_num > 1u) || gen_hdr) {
        pos += sizeof(seq_hdr_t) + sizeof(seq_entry_t) * seq_num;
    }
    /* pre act data block occupies 16 bytes  */
#define MAX_SEQ_SZ  (MAX_ACT_NUM*16)
    seq_buf = (char *)malloc(MAX_SEQ_SZ);

    for (int i = 0; i < seq_num; i++) {
        char *p_seq = seq_buf;

        seq_hdr->entry[i].tag = SEQ_ENTRY_TAG;
        seq_hdr->entry[i].start = pos;
        seq_hdr->entry[i].id = id[i];
        seq_hdr->entry[i].msk = msk[i];
        seq_hdr->entry[i].rsvd = 0;
        strncpy(seq_hdr->entry[i].name, file_i_name[i], sizeof(seq_hdr->entry[i].name));

        file_i = fopen(file_i_name[i], "r");

        if (NULL == file_i) {
            printf("Opps, failed to open %s\n", file_i_name[i]);
            res = -5;
            goto cleanup;
        }

        fseek(file_i, 0L, SEEK_END);
        size_t file_i_sz = ftell(file_i);
        fseek(file_i, 0L, SEEK_SET);

        buf = (char *)malloc(file_i_sz);

        if (NULL == buf) {
            res = -10;
            goto cleanup;
        }

        fread(buf, file_i_sz, 1, file_i);

        char *s = buf;
        char *e = buf;
        uint32_t line = 1;
        uint32_t act_cnt = 0;
        ddr_act_u last_act;
        char* annotation;

        while (1) {
            /* find the line end and end it with '\0'*/
            while (*e != '\n') {
                e++;
            }

            *e = '\0';

            if (act_cnt > MAX_ACT_NUM - 1) {
                printf("Number of seq shall less than %d\n", MAX_ACT_NUM);
                goto cleanup;
            }

            if (0 != strlen(s)) {
                /* to the valid line start */
                while (!(IS_HEX_CHAR(*s) || (*s == '/'))) {
                    s++;
                }

                if (!((*s == '/') && (*(s + 1) == '/'))) { /* not a comment */
                    /* When here, line started with a valid char */
#if DBG_LEVEL > 1
                    printf("line %d: %s\n", line, s);
#endif
                if (annotation = strstr(s, "//"))
                    *annotation = '\0';

                    bool wd_begin = false;
                    uint32_t argc = 0;
                    char *argv[MAX_ARGV_NUM];
                    char *p = s;

                    /* split the line into argvs */
                    while (*p != '\0') {
                        if (IS_HEX_CHAR(*p)
                            /* '_' '+' '*' and space between chars are allowed */
                            || (((*p == '_') || (*p == '+') || (*p == '*')
                                 || (*p == ' ')) && wd_begin)) {
                            if (!wd_begin) {
                                wd_begin = true;
                                argv[argc++] = p;
                            }
                        } else {
                            *p = '\0';
                            wd_begin = false;
                        }

                        p++;
                    }

                    if (argc >= 1) {
#if DBG_LEVEL > 1

                        for (int i = 0; i < argc; i++) {
                            printf("argv[%d] = %s\n", i, argv[i]);
                        }

                        printf("\n");
#endif

                        if (0 == strcmp(argv[0], "DDR_R32")) {
                        } else if (0 == strcmp(argv[0], "DDR_SEQ_BEGIN")) {
                        } else if (0 == strcmp(argv[0], "DDR_SEQ_END")) {
                        } else if (0 == strcmp(argv[0], "include ")) {
                        } else if (0 == strcmp(argv[0], "DDR_W32")) {
                            if (argc != 3) {
                                printf("Opps, wrong syntax at line %d, two paras expected but %d\n", line, argc - 1);
#if !defined(TEST)
                                goto cleanup;
#endif
                            }

                            ddr_wr_t wr;
                            memset(&wr, 0, sizeof(wr));
                            wr.act = ACT_WR;

                            if ((0 != cal_exp(argv[1], &(wr.addr))) ||
                                (0 != cal_exp(argv[2], &(wr.val)))) {
                                printf("Opps, invalid paras at line %d\n", line);
#if !defined(TEST)
                                goto cleanup;
#endif
                            }

#if DBG_LEVEL > 0
                            printf("ddr_wr_t: act=%x addr= 0x%x val=%x\n", wr.act, wr.addr, wr.val);
#endif
                            //fwrite(&wr, sizeof(wr), 1, file_o);
                            memcpy(p_seq, &wr, sizeof(wr));
                            p_seq += sizeof(wr);
                            act_cnt++;
                            memcpy(&last_act, &wr, sizeof(last_act));
                        } else if (0 == strcmp(argv[0], "DDR_W32_BITS")) {
                            if (argc != 5) {
                                printf("Opps, wrong syntax at line %d, 4 paras expected but %d\n", line, argc - 1);
#if !defined(TEST)
                                goto cleanup;
#endif
                            }

                            ddr_wr_bits_t act;
                            memset(&act, 0, sizeof(act));
                            act.act = ACT_WR_BITS;
                            uint32_t shift = 0, width = 0;

                            if ((0 != cal_exp(argv[1], &(act.addr)))
                                || (0 != cal_exp(argv[2], &shift))
                                || (0 != cal_exp(argv[3], &width))
                                || (0 != cal_exp(argv[4], &(act.val)))) {
                                printf("Opps, wrong paras at line %d\n", line);
#if !defined(TEST)
                                goto cleanup;
#endif
                            }

                            act.shift = shift;
                            act.width = width;
#if DBG_LEVEL > 0
                            printf("ddr_wr_bits: act=%x addr= 0x%x val=%x shift=%d width=%d\n",
                                   act.act, act.addr, act.val, act.shift, act.width);
#endif
                            //fwrite(&act, sizeof(act), 1, file_o);
                            memcpy(p_seq, &act, sizeof(act));
                            p_seq += sizeof(act);
                            act_cnt++;
                            memcpy(&last_act, &act, sizeof(last_act));

                        } else if (0 == strcmp(argv[0], "DDR_POLL_BITS")) {
                            if (argc != 5) {
                                printf("Opps, wrong syntax at line %d, 4 paras expected but %d\n", line, argc - 1);
#if !defined(TEST)
                                goto cleanup;
#endif
                            }

                            ddr_poll_bits_t act;
                            memset(&act, 0, sizeof(act));
                            act.act = ACT_POLL_BITS;
                            uint32_t tmt = 0;

                            if ((0 != cal_exp(argv[1], &(act.addr)))
                                || (0 != cal_exp(argv[2], &(act.msk)))
                                || (0 != cal_exp(argv[3], &(act.val)))
                                || (0 != cal_exp(argv[4], &tmt))) {
                                printf("Opps, wrong paras at line %d\n", line);
#if !defined(TEST)
                                goto cleanup;
#endif
                            }

                            act.tmt = (uint16_t)tmt;
                            act.tmt_h = (uint8_t)(tmt >> 16u);
#if DBG_LEVEL > 0
                            printf("ddr_poll_bits: act=%x addr= 0x%x val=%x msk=%d tmt=%d\n",
                                   act.act, act.addr, act.val, act.msk, tmt);
#endif
                            //fwrite(&act, sizeof(act), 1, file_o);
                            memcpy(p_seq, &act, sizeof(act));
                            p_seq += sizeof(act);
                            act_cnt++;
                            memcpy(&last_act, &act, sizeof(last_act));
                        } else if (0 == strcmp(argv[0], "DDR_INFO")) {
                            ddr_func_call_t call;
                            memset(&call, 0, sizeof(call));
                            call.act = ACT_FUNC_CALL;
                            call.func = FUNC_INFO;
#if DBG_LEVEL > 0
                            printf("ddr_func_call: act=%x func= 0x%x para[0]=%x para[1]=%d\n",
                                   call.act, call.func, call.para[0], call.para[1]);
#endif
                            //fwrite(&call, sizeof(call), 1, file_o);
                            memcpy(p_seq, &call, sizeof(call));
                            p_seq += sizeof(call);
                            act_cnt++;
                            memcpy(&last_act, &call, sizeof(last_act));
                        } else if (0 == strcmp(argv[0], "CFG_RST")) {
                            ddr_func_call_t call;
                            memset(&call, 0, sizeof(call));

                            if (0 != fill_func_act(argc, argv, 3, FUNC_RESET, line, &call)) {
                                res = -6;
                                goto cleanup;
                            }

                            //fwrite(&call, sizeof(call), 1, file_o);
                            memcpy(p_seq, &call, sizeof(call));
                            p_seq += sizeof(call);
                            act_cnt++;
                            memcpy(&last_act, &call, sizeof(last_act));
                        } else if (0 == strcmp(argv[0], "DDRPHY_WAITFWDONE")) {
                            ddr_func_call_t call;
                            memset(&call, 0, sizeof(call));

                            if (0 != fill_func_act(argc, argv, 2, FUNC_FW_MON, line, &call)) {
                                res = -7;
                                goto cleanup;
                            }
                            //fwrite(&call, sizeof(call), 1, file_o);
                            memcpy(p_seq, &call, sizeof(call));
                            p_seq += sizeof(call);
                            act_cnt++;
                            memcpy(&last_act, &call, sizeof(last_act));
                        } else if (0 == strcmp(argv[0], "LOAD_DDR_TRAINING_FW")) {
                            ddr_func_call_t call;
                            memset(&call, 0, sizeof(call));

                            if (0 != fill_func_act(argc, argv, 3, FUNC_LOADFW, line, &call)) {
                                res = -8;
                                goto cleanup;
                            }
                            //fwrite(&call, sizeof(call), 1, file_o);
                            memcpy(p_seq, &call, sizeof(call));
                            p_seq += sizeof(call);
                            act_cnt++;
                            memcpy(&last_act, &call, sizeof(last_act));
                        } else if (0 == strcmp(argv[0], "CFG_DDR_CLK")) {
                            ddr_func_call_t call;
                            memset(&call, 0, sizeof(call));

                            if (0 != fill_func_act(argc, argv, 2, FUNC_CFG_CLK, line, &call)) {
                                res = -9;
                                goto cleanup;
                            }

                            //fwrite(&call, sizeof(call), 1, file_o);
                            memcpy(p_seq, &call, sizeof(call));
                            p_seq += sizeof(call);
                            act_cnt++;
                            memcpy(&last_act, &call, sizeof(last_act));
                        } else if (0 == strcmp(argv[0], "dwc_ddrphy_phyinit_userCustom_H_readMsgBlock")) {
                            ddr_func_call_t call;
                            memset(&call, 0, sizeof(call));

                            if (0 != fill_func_act(argc, argv, 2, FUNC_GET_TRAIN_PARA, line, &call)) {
                                res = -11;
                                goto cleanup;
                            }
                            call.para[0] = 0;
                            //fwrite(&call, sizeof(call), 1, file_o);
                            memcpy(p_seq, &call, sizeof(call));
                            p_seq += sizeof(call);
                            act_cnt++;
                            memcpy(&last_act, &call, sizeof(last_act));
                        } else if ((0 == strcmp(argv[0], "timing_reg_update_after_training"))
                                   || (0 == strcmp(argv[0], "timing_reg_update_after_training_ddr4"))) {
                            ddr_func_call_t call;
                            memset(&call, 0, sizeof(call));

                            if (0 != fill_func_act(argc, argv, 1, FUNC_USE_TRAIN_PARA, line, &call)) {
                                res = -12;
                                goto cleanup;
                            }
                            //fwrite(&call, sizeof(call), 1, file_o);
                            memcpy(p_seq, &call, sizeof(call));
                            p_seq += sizeof(call);
                            act_cnt++;
                            memcpy(&last_act, &call, sizeof(last_act));
                        } else if (0 == strcmp(argv[0], "DDR_SEQ_VERSION_PRINT")) {
                            ddr_func_call_t call;
                            memset(&call, 0, sizeof(call));

                            if (0 != fill_func_act(argc, argv, 2, FUNC_PRINT_SEQ_VERSION, line, &call)) {
                                res = -13;
                                goto cleanup;
                            }
                            //fwrite(&call, sizeof(call), 1, file_o);
                            memcpy(p_seq, &call, sizeof(call));
                            p_seq += sizeof(call);
                            act_cnt++;
                            memcpy(&last_act, &call, sizeof(last_act));
                        }else if (0 == strcmp(argv[0], "DDR_UDELAY")) {
                            ddr_func_call_t call;
                            memset(&call, 0, sizeof(call));
                            call.act = ACT_FUNC_CALL;
                            call.func = FUNC_UDELAY;
                            
                            if (0 != cal_exp(argv[1], &call.para[0])){
                                printf("Opps, wrong paras at line %d\n", line);
                            }
#if DBG_LEVEL > 0
                            printf("ddr_func_call: act=%x func= 0x%x para[0]=%x para[1]=%d\n",
                                   call.act, call.func, call.para[0], call.para[1]);
#endif
                            memcpy(p_seq, &call, sizeof(call));
                            p_seq += sizeof(call);
                            act_cnt++;
                            memcpy(&last_act, &call, sizeof(last_act));
                        } else if (0 == strcmp(argv[0], "RUN_DIAG")) {
                            ddr_func_call_t call;
                            memset(&call, 0, sizeof(call));

                            if (0 != fill_func_act(argc, argv, 2, FUNC_DIAG, line, &call)) {
                                res = -14;
                                goto cleanup;
                            }
#if DBG_LEVEL > 0
                            printf("ddr_func_call: act=%x func= 0x%x para[0]=%x para[1]=%d\n",
                                   call.act, call.func, call.para[0], call.para[1]);
#endif
                            memcpy(p_seq, &call, sizeof(call));
                            p_seq += sizeof(call);
                            act_cnt++;
                            memcpy(&last_act, &call, sizeof(last_act));
                        } else {
                            printf("Opps, %s not supported\n", argv[0]);  
#if !defined(TEST)
                            res = -14;
                            goto cleanup;
#endif
                        }
                    }
                }
            }

            /* to next line */
            s = e + 1;
            e = e + 1;
            line++;

            if (s >= (buf + file_i_sz)) {
                break;
            }
        }

        if (act_cnt > 0) {
            ddr_func_call_t *call = (ddr_func_call_t *)(&last_act);

            if (call->act != ACT_END) {
                if (act_cnt > MAX_ACT_NUM - 1) {
                    printf("Opps, no space to add SEQ_END\n");
                    goto cleanup;
                }

#if DBG_LEVEL > 0
                printf("Add SEQ_END...\n");
#endif
                memset(&last_act, 0, sizeof(last_act));
                call->act = ACT_END;
                //fwrite(call, sizeof(ddr_func_call_t), 1, file_o);
                memcpy(p_seq, call, sizeof(ddr_func_call_t));
                p_seq += sizeof(ddr_func_call_t);
            }
        }

        uint32_t size = p_seq - seq_buf;
        seq_hdr->entry[i].size = size;
        seq_hdr->entry[i].seq_crc32 = cal_crc32(seq_buf, size);
        seq_hdr->entry[i].crc32 = cal_crc32(&(seq_hdr->entry[i]), sizeof(seq_hdr_t) - 4);
        pos += size;
        fwrite(seq_buf, 1, size, file_o);

        fclose(file_i);
        file_i = NULL;
        free(buf);
        buf = NULL;
    }

    if ((seq_num > 1u) || gen_hdr) {
        fseek(file_o, 0L, SEEK_SET);
        fwrite(seq_hdr, 1, sizeof(seq_hdr_t) + sizeof(seq_entry_t) * seq_num, file_o);
    }

cleanup:

    if (NULL != buf) free(buf);

    if (NULL != seq_buf) free(seq_buf);

    if (NULL != seq_hdr) free(seq_hdr);

    if (NULL != file_i) fclose(file_i);

    if (NULL != file_o) fclose(file_o);

    return res;
}
