/*
 * SEMIDRIVE Copyright Statement
 * Copyright (c) SEMIDRIVE. All rights reserved
 *
 * This software and all rights therein are owned by SEMIDRIVE, and are
 * protected by copyright law and other relevant laws, regulations and
 * protection. Without SEMIDRIVE's prior written consent and/or related rights,
 * please do not use this software or any potion thereof in any form or by any
 * means. You may not reproduce, modify or distribute this software except in
 * compliance with the License. Unless required by applicable law or agreed to
 * in writing, software distributed under the License is distributed on
 * an "AS IS" basis, WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * You should have received a copy of the License along with this program.
 * If not, see <http://www.semidrive.com/licenses/>.
 */

#include "update.h"
#include "sparse_parser.h"

typedef struct PACKET_HEADER
{
    uint32_t nMagic;           //(0x5041434B) which means "PACK"
    uint32_t nVersion;         //0
    uint8_t szPrdName[256];    // product name
    uint8_t szPrdVersion[256]; // product version
    uint32_t nPreLoadSize;     // spl or safety size
    uint32_t nPreLoadOffset;   //spl or safety offset from the packet header and the array of FILE_T struct buffer to preload file
    uint32_t nFileTCount;      // the number of PACKET_FILE_T
    uint32_t nFileTOffset;     // the offset from the packet file header to the array of FILE_T struct buffer
    uint8_t bReserved[484];    // reserved
    uint32_t nCRC32;           // CRC 32 for all data except  nCRC
} PACKET_HEADER_T;

const int PACKET_HEADER_T_SIZE = sizeof(PACKET_HEADER_T);    // Length = 1KB

typedef struct PACKET_FILE
{
    uint8_t szPartitionName[72]; // partition name
    uint64_t llSize;             // file size
    uint64_t llOffset;           // data offset
    uint32_t nFileType;          //0:DA; 1:main gpt; 2:image; 3:secondary gpt; 4:pac;
    uint8_t bReserved[36];       // reserved
} PACKET_FILE_T;

const int PACKET_FILE_T_SIZE = sizeof(PACKET_FILE_T);    // Length = 128

typedef struct PAC_IMG_INFO
{
    char img_name[72];           // partition name
    bool is_sparse;
} PAC_IMG_INFO_T;

typedef struct UPDATE_PAC_DEV
{
    partition_device_t *ptdev;
    PACKET_HEADER_T pack_head_t;
    PACKET_FILE_T pack_file_t[64];
    int slot_a[64];
    int slot_b[64];
    int update_partition[64];
    int nImageCount;
    uint64_t total_size;
    uint64_t cur_size;
    int percent;
    PAC_IMG_INFO_T img_info[64];
} UPDATE_PAC_DEV_T;

UPDATE_PAC_DEV_T emmc_pac_dev,ospi_pac_dev;


void wchar2char(uint8_t* src,uint8_t* des,int len){
    int index = 0;
    for(int i= 0;i< len;i++){
        if(i%2 == 0){
            des[index] = src[i];
            index ++;
        }
    }
}

UPDATE_PAC_DEV_T *get_pac_dev(int type)
{
    if(type == OSPI_TYPE){
        return &ospi_pac_dev;
    }else if(type == EMMC_TYPE){
        return &emmc_pac_dev;
    }
    return NULL;
}

int dump_update_progress(int type,int add_size){
    if(type > 1 || type < 0 || add_size <0){
        PRINTF_CRITICAL("failed dump_update_progress for invilad type or add_size\n");
        return -1;
    }
    UPDATE_PAC_DEV_T *pac_dev = get_pac_dev(type);
    if(!pac_dev){
        return -1;
    }
    int c = pac_dev->percent;
    pac_dev->cur_size += add_size;
    pac_dev->percent = (100*pac_dev->cur_size/pac_dev->total_size);
    if(c < pac_dev->percent)
        PRINTF_INFO("%s update progress %d%%\n",type == EMMC_TYPE ? "EMMC":"OSPI",pac_dev->percent);
    if(pac_dev->percent == 100){
        PRINTF_INFO("%s update success!\n",type == EMMC_TYPE ? "EMMC":"OSPI");
    }
    return 0;
}

int dump_partition_info(int type){
    UPDATE_PAC_DEV_T *pac_dev = get_pac_dev(type);
    if(!pac_dev){
        PRINTF_CRITICAL("failed dump_partition_info for pac_dev is NULL\n");
        return -1;
    }
    ptdev_attr_dump(pac_dev->ptdev);
    ptdev_dump(pac_dev->ptdev);
    return 0;
}

int get_stuff_state(const char*name,char*stuff){
    if(name){
        int len = strlen(name);
        if(len > 2 && name[len -2] == stuff[0] && name[len-1] == stuff[1]){
            return 1;
        }
    }
    return 0;
}

int get_need_update_slot(type){
    if(type > 1 || type < 0){
        PRINTF_CRITICAL("failed make_boot_slot for invilad type\n");
        return -1;
    }
    UPDATE_PAC_DEV_T *pac_dev = get_pac_dev(type);
    if(!pac_dev){
        return -1;
    }
    int slot;
    const char *suffix;
    slot = get_current_slot(pac_dev->ptdev);
    suffix = get_suffix(pac_dev->ptdev, slot);
    return get_stuff_state(suffix,"_a") == 1 ? SLOT_B:SLOT_A;
}

int make_boot_slot(int type){
    if(type > 1 || type < 0){
        PRINTF_CRITICAL("failed make_boot_slot for invilad type\n");
        return -1;
    }
    UPDATE_PAC_DEV_T *pac_dev = get_pac_dev(type);
    if(!pac_dev){
        return -1;
    }
    const char *suffix;
    int slot,ret,inactive_slot;
    dump_partition_info(type);
    slot = get_current_slot(pac_dev->ptdev);
    ret = get_number_slots(pac_dev->ptdev);
    PRINTF_INFO("slots count %d\n", ret);
    if (slot == INVALID) {
        PRINTF_CRITICAL("bad slot %d\n", slot);
        return -1;
    }

    suffix = get_suffix(pac_dev->ptdev, slot);
    PRINTF_INFO("slot suffix %s\n", suffix);

    inactive_slot = get_inverse_slot(pac_dev->ptdev, slot);

    /* mark bootup successfully if necessary */
    ret = is_slot_marked_successful(pac_dev->ptdev, slot);

    if (ret != 1) {
        mark_boot_successful(pac_dev->ptdev);
    }
    ptdev_clean_slot_attr(pac_dev->ptdev, inactive_slot, ATTR_SUCCESSFUL);
    /* retry */
    ptdev_mark_slot_attr(pac_dev->ptdev, inactive_slot, ATTR_RETRY);

    /* active & bootable */
    set_active_boot_slot(pac_dev->ptdev, inactive_slot);
    dump_partition_info(type);
    sync();
    return 0;
}

int read_img_file_from(const char*name,int from,int len, uint8_t*data){
    int ret;
    FILE *fp;
    char path[256];
    sprintf(path,"%s%s",EMMC_PAC_ROOT_PATH,name);
    if ((fp = fopen(path, "rb+")) == NULL){
        PRINTF_CRITICAL("read_img_file_from Cannot open file %s, return!\n",path);
        return -1;
    }
    fseek(fp, from, SEEK_SET);
    ret = fread(data, 1, len, fp);
    fclose(fp);
    if (ret == len)return 0;
    else {
        PRINTF_CRITICAL("failed to read_img_file_from %s ret %d len %d\n",name,ret,len);
        return -1;
    }
}


int init_dev(int type){
    int ret = -1;
    switch(type){
        case EMMC_TYPE:
            memset(&emmc_pac_dev,0,sizeof(UPDATE_PAC_DEV_T));
            ret = switch_storage_dev(&emmc_pac_dev.ptdev, DEV_EMMC0);
            if (0 != ret) {
                PRINTF_CRITICAL("failed to init emmc dev\n");
                return -1;
            }
            return 0;
        case OSPI_TYPE:
            memset(&ospi_pac_dev,0,sizeof(UPDATE_PAC_DEV_T));
            ret = switch_storage_dev(&ospi_pac_dev.ptdev, DEV_SPI_NOR0);
            if (0 != ret) {
                PRINTF_CRITICAL("failed to init ospi dev\n");
                return -1;
            }
            return 0;
    }
    return -1;
}

int decode_gtp_head(int type){
    int ret;
    if(type > 1 || type < 0){
        PRINTF_CRITICAL("failed decode_gtp_head for invilad type\n");
        return -1;
    }
    UPDATE_PAC_DEV_T *pac_dev = get_pac_dev(type);
    if(!pac_dev){
        return -1;
    }
    uint8_t name[256];
    uint8_t version[256];
    uint8_t buffer[PACKET_HEADER_T_SIZE];
    ret = read_img_file_from(type  == EMMC_TYPE ? GLOBAL_PAC_NAME : OSPI_PAC_NAME,0,PACKET_HEADER_T_SIZE,buffer);
    if (0 != ret) {
        PRINTF_CRITICAL("failed to decode_gtp_head\n");
        return -1;
    }
    memcpy(&pac_dev->pack_head_t,buffer,PACKET_HEADER_T_SIZE);
    wchar2char(pac_dev->pack_head_t.szPrdName,name,256);
    wchar2char(pac_dev->pack_head_t.szPrdVersion,version,256);
    PRINTF_INFO("decode_gtp_head EMMC szPrdName %s szPrdVersion %s nPreLoadSize 0x%x nPreLoadOffset 0x%x nFileTCount %d nFileTOffset 0x%x",
    name,version,pac_dev->pack_head_t.nPreLoadSize,pac_dev->pack_head_t.nPreLoadOffset,
    pac_dev->pack_head_t.nFileTCount,pac_dev->pack_head_t.nFileTOffset);
    return 0;
}

int decode_gtp_entry(int type){
    int ret;
    if(type > 1 || type < 0){
        PRINTF_CRITICAL("failed decode_gtp_entry for invilad type\n");
        return -1;
    }
    UPDATE_PAC_DEV_T *pac_dev = get_pac_dev(type);
    if(!pac_dev){
        PRINTF_CRITICAL("failed decode_gtp_entry for pac_dev is NULL\n");
        return -1;
    }
    if (pac_dev->pack_head_t.nFileTCount <= 0) {
        PRINTF_CRITICAL("failed to decode_gtp_entry for invilad nFileTCount\n");
        return -1;
    }
    uint8_t buffer[PACKET_FILE_T_SIZE*pac_dev->pack_head_t.nFileTCount];
    ret = read_img_file_from(type  == EMMC_TYPE ? GLOBAL_PAC_NAME : OSPI_PAC_NAME,PACKET_HEADER_T_SIZE,PACKET_FILE_T_SIZE*pac_dev->pack_head_t.nFileTCount,buffer);
    if (0 != ret) {
        PRINTF_CRITICAL("failed to decode_gtp_entry for read\n");
        return -1;
    }
    for (int i= 0;i< pac_dev->pack_head_t.nFileTCount;i++){
        uint8_t name[MAX_GPT_NAME_SIZE];
        memcpy(&pac_dev->pack_file_t[i],buffer+i*PACKET_FILE_T_SIZE,PACKET_FILE_T_SIZE);
        wchar2char(pac_dev->pack_file_t[i].szPartitionName,name,MAX_GPT_NAME_SIZE);
        sprintf(pac_dev->img_info[i].img_name,"%s",name);
        PRINTF_INFO("decode_gtp_entry szPartitionName %s llSize 0x%lx llOffset 0x%lx nFileType %d\n",pac_dev->img_info[i].img_name,pac_dev->pack_file_t[i].llSize
        ,pac_dev->pack_file_t[i].llOffset,pac_dev->pack_file_t[i].nFileType);
    }
    return 0;
}

int separate_a_b_slot(int type){
    if(type > 1 || type < 0){
        PRINTF_CRITICAL("failed decode_gtp_head for invilad type\n");
        return -1;
    }
    UPDATE_PAC_DEV_T *pac_dev = get_pac_dev(type);
    if(!pac_dev){
        PRINTF_CRITICAL("failed separate_a_b_slot for pac_dev is NULL\n");
        return -1;
    }
    int j = 0;
    int k = 0;
    for(int i=0;i< pac_dev->ptdev->count;i++){
        if(get_stuff_state((char*)pac_dev->ptdev->partition_entries[i].name,"_a")){
            pac_dev->slot_a[j] = i;
            PRINTF_INFO("separate_a_b_slot slot_a[%d]= %d name = %s\n", j,i,pac_dev->ptdev->partition_entries[i].name);
            j++;
            pac_dev->nImageCount++;
            }else if(get_stuff_state((char*)pac_dev->ptdev->partition_entries[i].name,"_b")){
            pac_dev->slot_b[k] = i;
            PRINTF_INFO("separate_a_b_slot slot_b[%d]= %d name = %s\n", k,i,pac_dev->ptdev->partition_entries[i].name);
            k++;
        }
    }
    return 0;
}

int select_update_partition(int type){
    if(type > 1 || type < 0){
        PRINTF_CRITICAL("failed decode_gtp_head for invilad type\n");
        return -1;
    }
    UPDATE_PAC_DEV_T *pac_dev = get_pac_dev(type);
    if(!pac_dev){
        PRINTF_CRITICAL("failed separate_a_b_slot for pac_dev is NULL\n");
        return -1;
    }
    int k = 0;
    for (int i= 0;i< pac_dev->pack_head_t.nFileTCount;i++){
        for(int j = 0;j< pac_dev->nImageCount;j++){
            int len = strlen((char *)pac_dev->ptdev->partition_entries[pac_dev->slot_a[j]].name);
            int len1 = strlen((char*)pac_dev->img_info[i].img_name);
            if(strncmp((char*)pac_dev->ptdev->partition_entries[pac_dev->slot_a[j]].name,(char*)pac_dev->img_info[i].img_name,len-2) == 0 && len == len1){
                pac_dev->update_partition[k] = i;
                PRINTF_INFO("update_partition[%d]= %d name = %s\n", k,i,pac_dev->img_info[i].img_name);
                if(is_sparse_img(type,pac_dev->pack_file_t[pac_dev->update_partition[k]].llOffset)){
                    pac_dev->total_size += get_sparse_img_size(type,pac_dev->pack_file_t[pac_dev->update_partition[k]].llOffset);
                }else{
                    pac_dev->total_size += pac_dev->pack_file_t[pac_dev->update_partition[k]].llSize;
                }
                k++;
            }
        }
    }
    return 0;
}

int update_partition(int type,int slot_partition){
    UPDATE_PAC_DEV_T *pac_dev = get_pac_dev(type);
    if(!pac_dev){
        PRINTF_CRITICAL("failed update_partition for pac_dev is NULL\n");
        return -1;
    }
    storage_device_t *storage = pac_dev->ptdev->storage;
    int ret;
    unsigned long long size = 0;
    unsigned long long rsize = 0;
    unsigned long long in = 0;
    unsigned long long out = 0;
    unsigned long long left = 0;
    unsigned long long cnt = 0;
    uint8_t *data;
    int j = 0;
    for(int i=0;i< pac_dev->ptdev->count;i++){
        if(i == (slot_partition == SLOT_A ? pac_dev->slot_a[j]: pac_dev->slot_b[j])){
            rsize = (pac_dev->ptdev->partition_entries[i].last_lba -
            pac_dev->ptdev->partition_entries[i].first_lba + 1) * LBA_SIZE;
            out = (pac_dev->ptdev->partition_entries[i].first_lba) * LBA_SIZE;
            out += pac_dev->ptdev->gpt_offset;
            in = pac_dev->pack_file_t[pac_dev->update_partition[j]].llOffset;
            size = pac_dev->pack_file_t[pac_dev->update_partition[j]].llSize;
            PRINTF_INFO("i=%d\n", i);
            PRINTF_INFO("update_emmc_partion=%s\n", pac_dev->ptdev->partition_entries[i].name);
            PRINTF_INFO("in=%p out=%p rsize=0x%llx size=0x%llx\n",(void*)in,(void *)out,rsize,size);
            PRINTF_INFO("update_emmc_partion szPartitionName %s llSize 0x%lx llOffset 0x%lx nFileType %d\n",pac_dev->img_info[pac_dev->update_partition[j]].img_name,pac_dev->pack_file_t[pac_dev->update_partition[j]].llSize
            ,pac_dev->pack_file_t[pac_dev->update_partition[j]].llOffset,pac_dev->pack_file_t[pac_dev->update_partition[j]].nFileType);
            int len = strlen((char *)pac_dev->ptdev->partition_entries[i].name);
            if(strncmp((char*)pac_dev->ptdev->partition_entries[i].name,(char*)pac_dev->img_info[pac_dev->update_partition[j]].img_name,len-2) != 0){
                PRINTF_CRITICAL("update_emmc_partion check pac data failed\n");
                return -1;
            }else{
                uint8_t sdata[4];
                read_img_file_from(type  == EMMC_TYPE ? GLOBAL_PAC_NAME : OSPI_PAC_NAME,in,4,sdata);
                if(sdata[0] == 0x3A && sdata[1] == 0xFF && sdata[2] == 0x26 && sdata[3] == 0xED){//sparse img
                    PRINTF_INFO("update_partition %s is sparse img need transto raw\n", pac_dev->img_info[pac_dev->update_partition[j]].img_name);
                    flash_sparse_img(type,in,out,rsize,pac_dev->img_info[pac_dev->update_partition[j]].img_name,storage);
                }else{
                    cnt =  (size / MAX_CALLOC_SIZE);
                    left = (size % MAX_CALLOC_SIZE);
                    data = (unsigned char *)calloc(1, (cnt != 0 ? (MAX_CALLOC_SIZE) : size));

                    if (!data) {
                        PRINTF_CRITICAL("update_emmc_partion calloc failed\n");
                        return -1;
                    }

                    for ( unsigned long long n = 0; n < cnt; n++) {
                        ret = read_img_file_from(type  == EMMC_TYPE ? GLOBAL_PAC_NAME : OSPI_PAC_NAME,in,MAX_CALLOC_SIZE,data);
                        dump_update_progress(type,MAX_CALLOC_SIZE);
                        if (ret < 0) {
                            PRINTF_CRITICAL("update_emmc_partion read_emmc_file_form failed\n");
                            free(data);
                            return -1;
                        }
                        ret = storage->write(storage, out, data, MAX_CALLOC_SIZE);

                        if (ret < 0) {
                            PRINTF_CRITICAL("update_emmc_partion storage->write failed\n");
                            free(data);
                            return -1;
                        }

                        in += MAX_CALLOC_SIZE;
                        out += MAX_CALLOC_SIZE;
                    }

                    if (0 != left) {
                        read_img_file_from(type  == EMMC_TYPE ? GLOBAL_PAC_NAME : OSPI_PAC_NAME,in,left,data);
                        dump_update_progress(type,left);
                        ret = storage->write(storage, out, data, left);

                        if (ret < 0) {
                            PRINTF_CRITICAL("update_emmc_partion storage->write failed\n");
                            free(data);
                            return -1;
                        }
                    }
                }
            }
            j++;
        }
    }
    return 0;
}


int deinit_dev(int type){
    UPDATE_PAC_DEV_T *pac_dev = get_pac_dev(type);
    if(!pac_dev){
        PRINTF_CRITICAL("failed deinit_dev for pac_dev is NULL\n");
        return -1;
    }
    ptdev_destroy(pac_dev->ptdev);
    return 0;
}

int do_update(int type) {
    int ret;

    ret = init_dev(type);
    if(ret != 0){
        PRINTF_CRITICAL("failed init_dev\n");
    }

    ret = decode_gtp_head(type);
    if(ret != 0){
        PRINTF_CRITICAL("failed decode_gtp_head\n");
    }

    ret = decode_gtp_entry(type);
    if(ret != 0){
        PRINTF_CRITICAL("failed decode_gtp_entry\n");
    }

    ret = separate_a_b_slot(type);
    if(ret != 0){
        PRINTF_CRITICAL("failed separate_a_b_slot\n");
    }

    ret = select_update_partition(type);
    if(ret != 0){
        PRINTF_CRITICAL("failed select_update_partition\n");
    }

    ret = update_partition(type,get_need_update_slot(type));
    if(ret != 0){
        PRINTF_CRITICAL("failed update_partition\n");
    }

    make_boot_slot(type);
    /*ret = deinit_dev(type);
    if(ret != 0){
        PRINTF_CRITICAL("failed deinit_dev\n");
    }*/
    return 0;
}
