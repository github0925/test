/*
 * ddr_seq_parser.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _DDR_SEQ_PARSER_H_
#define _DDR_SEQ_PARSER_H_

#include <stdint.h>

#define MAX_SEQ_NUM     8
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

#endif
