#include <arch.h>
#include <debug.h>
#include <malloc.h>
#include <cksum.h>

#include "fuse_ctrl.h"
#include "internal.h"
#include "libavb.h"
#include <ab_partition_parser.h>
#include "partition_parser.h"
#include "storage_device.h"
#include "sd_x509.h"
#include "verified_boot.h"

#define DEBUG_ON 0
#define ROOT_CA_OFFSET     0x9C
#define ROOT_CA_SIZE       0x414
#define ROOT_CA_TAG        0xEAF0
#define PK_CA_OFFSET       0x10
#define PK_TYPE_CA_OFFSET  0x5
#define ROOT_CA_SECURITY   0x1
#define ROOT_CA_SAFETY     0x2

#define PK_RSA_1024        0x03
#define PK_RSA_2048        0x04
#define PK_RSA_3072        0x05
#define PK_RSA_4096        0x06
#define PK_RSA_N_OFF       0x04
#define PK_RSA_E_OFF       0x204
#define PK_RSA_MAX_LEN     0x200

#define PK_EC_P256V1       0x12
#define PK_EC_SECP384R1    0x13
#define PK_EC_SECP521R1    0x14
#define PK_EC_QX_OFF       0x04
#define PK_EC_QY_OFF       0x44
#define PK_EC_Q_MAX_LEN    0x44

#define BPT_TAG            0x42505401
#define BPT_CHKSUM_OFFSET  0x7FC

#define IIB_TAG            0xEAE1
#define IIB_OFFSET         0x20
#define IMG_SZ_IIB_OFFSET  0x28

/* exponent of public key of rsa in libavb
 * is always 0x10001
 * */
#define AVB_RSA_PK_E       {0x1, 0x00, 0x01}

#define ROTPK0_FUSE_INDEX   0x24
#define ROTPK1_FUSE_INDEX   0x2C
#define RMV_FUSE_INDEX      0xB5
#define RMV_FUSE_SIZE_BYTE  28
#define SEC_ROTPK_FUSE_INDEX ROTPK0_FUSE_INDEX
#define SAF_ROTPK_FUSE_INDEX ROTPK1_FUSE_INDEX
#define PROD_FUSE_INDEX     0xA9
#define PROD_FUSE_BIT       7
#define DEFAULT_VBMETA_UUID "20818d08-6339-474a-b119-7eea0b60ac3c"

#if IN_SAFETY_DOMAIN
#define ROTPK_FUSE_INDEX SAF_ROTPK_FUSE_INDEX
#else
#define ROTPK_FUSE_INDEX SEC_ROTPK_FUSE_INDEX
#endif

#define SHA256_DIGEST_LENGTH 32
#define LOCAL_TRACE 0
#define LTRACEF(x...) do { if (LOCAL_TRACE) { WARN(x);  }  } while (0)

#define ERROR(format, args...) WARN(\
                               "ERROR:%s %d "format"\n", __func__, __LINE__,  ##args);

#if DEBUG_ON
#define DEBUG_DUMP(ptr, size, format, args...) \
    do{ \
        dprintf(CRITICAL, "%s %d "format"\n", __func__, __LINE__, ##args); \
        hexdump8(ptr, size); \
    }while(0);
#else
#define DEBUG_DUMP(ptr, size, format, args...)
#endif

typedef struct {
    uint32_t  data1;
    uint16_t  data2;
    uint16_t  data3;
    uint8_t   data4[8];
} EFI_GUID;

struct root_key {
    uint8_t *n;
    uint8_t *e;
    uint32_t len;
};

char *strdup(const char *str)
{
    size_t len;
    char *copy;

    len = strlen(str) + 1;
    copy = calloc(1, len);

    if (copy == NULL)
        return NULL;

    memcpy(copy, str, len);
    return copy;
}

static bool is_current_slot_successful(partition_device_t *ptdev)
{
    struct ab_slot_info slot_info[AB_SUPPORTED_SLOTS];
    int slot_idx = INVALID;

    slot_idx = ptdev_find_active_slot(ptdev);

    if (slot_idx == INVALID) {
        ERROR("IsCurrentSlotSuccessful: no active slots found!\n");
        return false;
    }

    ptdev_fill_slot_meta(ptdev, slot_info);

    if (!strncmp(slot_info[slot_idx].slot_is_succesful_rsp, "Yes",
                 strlen("Yes")))
        return true;

    return false;
}

AvbIOResult avb_get_preload_partition(AvbOps *ops, const char *partition,
                                      size_t num_bytes, uint8_t **out_pointer,
                                      size_t *out_num_bytes_preloaded)
{
    AvbOpsUserData *user_data = NULL;
    AvbIOResult ret = AVB_IO_RESULT_OK;
    struct image_load_info *img_info = NULL;
    addr_t img_addr = 0;

    if (!partition || !out_pointer
            || !out_num_bytes_preloaded || num_bytes <= 0) {
        ERROR("bad input paramaters\n");
        goto out;
    }

    user_data = ops->user_data;
    *out_num_bytes_preloaded = 0;
    *out_pointer = NULL;

    LTRACEF("get preloaded image, name:%s \n", partition);
    list_for_every_entry(&(user_data->preload_head), img_info,
                         struct image_load_info, node) {
        if (!strcmp(img_info->name, partition)) {
            if ( num_bytes <= img_info->size ) {
                img_addr = (addr_t)img_info->addr;
                *out_pointer = (uint8_t *)img_addr;
                *out_num_bytes_preloaded = num_bytes;
                ret = AVB_IO_RESULT_OK;
            }
            else {
                ERROR("preload image size is not match, size:%llu num_bytes:%llu partition:%s",
                      img_info->size, (uint64_t)num_bytes, partition);
            }

            break;
        }
    }

out:
    return ret;
}

static AvbIOResult read_data_from_partition(partition_device_t  *ptdev,
        const char *partition,
        int64_t offset, size_t num_bytes,
        void *buffer, size_t *out_num_read)
{
    AvbIOResult ret = AVB_IO_RESULT_OK;
    storage_device_t *storage = NULL;
    uint64_t ptn = 0;
    int read_status = 0;
    uint32_t block_size = 0;
    uint64_t pt_size = 0;
    uint64_t read_offset = 0;
    uint8_t *block_buffer = NULL;
    uint64_t unaligned_buf_addr = 0;
    uint64_t unaligned_offset = 0;
    uint64_t unaligned_num_bytes = 0;
    uint64_t aligned_read_num_bytes = 0;
    uint8_t *buffer_aligned = NULL;
    uint32_t len_align_req    = 0;
    uint32_t offset_align_req = 0;
    uint32_t buffer_align_req = 0;

    if (!ptdev || !ptdev->storage) {
        ERROR("partition ptn or size error!\n");
        ret = AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;
        goto out;

    }

    *out_num_read = 0;
    storage = ptdev->storage;

    ptn = ptdev_get_offset(ptdev, partition);
    pt_size = ptdev_get_size(ptdev, partition);

    if (!ptn || !pt_size) {
        ERROR("partition ptn or size error, partition:%s!\n", partition);
        ret = AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;
        goto out;
    }

    if (offset < 0) {
        if ((-offset) > (int64_t)pt_size) {
            ERROR("Negative Offset outside range.\n");
            ret = AVB_IO_RESULT_ERROR_RANGE_OUTSIDE_PARTITION;
            goto out;
        }

        read_offset = pt_size - (-offset);
    }
    else {
        read_offset = offset;
    }

    if (read_offset >= pt_size) {
        ERROR("Offset outside range.\n");
        ret = AVB_IO_RESULT_ERROR_RANGE_OUTSIDE_PARTITION;
        goto out;
    }

    if (num_bytes > pt_size - read_offset) {
        num_bytes = pt_size - read_offset;
    }

    LTRACEF(
        "read from %s, 0x%llx bytes at Offset 0x%llx, partition size %llu\n",
        partition, (uint64_t)num_bytes, (int64_t)offset, (uint64_t)pt_size);

    block_size = storage->get_block_size(storage);

    /* offset_align_req,
     * buffer_align_req,
     * and len_align_req may not be equal.
     * Here, they are equal.
     * */
    offset_align_req = block_size;
    buffer_align_req = block_size;
    len_align_req    = block_size;

    buffer_aligned = (uint8_t *)(addr_t)round_up((addr_t)buffer,
                     buffer_align_req);
    unaligned_buf_addr = (uint64_t)(buffer_aligned - (uint8_t *)buffer);

    unaligned_offset  = read_offset;
    read_offset       = round_down(read_offset, offset_align_req);
    unaligned_offset -= read_offset;

    /* The length of buffer is num_bytes bytes.
     * Make sure that access of the buffer isn't out of bounds.
     * */
    if (num_bytes < len_align_req || num_bytes < unaligned_buf_addr) {
        aligned_read_num_bytes = 0;
    }
    else {
        aligned_read_num_bytes = num_bytes - unaligned_buf_addr;
        aligned_read_num_bytes = round_down(aligned_read_num_bytes, len_align_req);
    }

    if (aligned_read_num_bytes) {
        LTRACEF("\nread from 0x%llx to 0x%p size:0x%llx\n", read_offset,
                buffer_aligned, aligned_read_num_bytes);

        read_status = storage->read(storage, ptn + read_offset, buffer_aligned,
                                    aligned_read_num_bytes);

        if (read_status) {
            ERROR("storage read error!\n");
            ret = AVB_IO_RESULT_ERROR_IO;
            goto out;
        }

        if (unaligned_offset || unaligned_buf_addr) {
            LTRACEF("\nactual move from 0x%p to 0x%p size:0x%llx\n",
                    buffer_aligned + unaligned_offset, buffer,
                    aligned_read_num_bytes - unaligned_offset);

            /* Because of the src overlapping with the dst,
             * uses 'memmove' to copy data for safety.
             * And skip useless data
             * */
            memmove(buffer, buffer_aligned + unaligned_offset,
                    aligned_read_num_bytes - unaligned_offset);
        }
        else {
            LTRACEF("don't need move\n");
        }

        *out_num_read = aligned_read_num_bytes - unaligned_offset;

        /* Because unaligned_offset bytes invalid data have been read,
        * needs to read unaligned_offset bytes valid data
        * */
        unaligned_num_bytes = num_bytes - aligned_read_num_bytes
                              + unaligned_offset;

        if (unaligned_num_bytes) {
            LTRACEF("\nread again from:0x%llx  size:0x%llx  actual read size:0x%llx\n",
                    read_offset + aligned_read_num_bytes, unaligned_num_bytes,
                    round_up(unaligned_num_bytes, len_align_req));

            block_buffer = memalign(buffer_align_req,
                                    round_up(unaligned_num_bytes, len_align_req));

            if (block_buffer == NULL) {
                ERROR("Allocate for partial read failed!");
                ret = AVB_IO_RESULT_ERROR_OOM;
                goto out;
            }

            read_status = storage->read(storage,
                                        ptn + read_offset + aligned_read_num_bytes,
                                        block_buffer, round_up(unaligned_num_bytes, len_align_req));

            if (read_status) {
                ERROR("storage read error!\n");
                ret = AVB_IO_RESULT_ERROR_IO;
                goto out;
            }

            LTRACEF("\nmove again from:xxx to 0x%p size:0x%llx\n",
                    buffer + aligned_read_num_bytes - unaligned_offset,
                    unaligned_num_bytes);

            /* Here, src doesn't overlap with dst,
             * so using memcpy.
             * Otherwise, memmove should be used.
             * */
            memcpy(buffer + aligned_read_num_bytes - unaligned_offset,
                   block_buffer, unaligned_num_bytes);
            *out_num_read += unaligned_num_bytes;
        }
    }
    else {
        unaligned_num_bytes = num_bytes + unaligned_offset;

        block_buffer = memalign(buffer_align_req,
                                round_up(unaligned_num_bytes, len_align_req));

        if (block_buffer == NULL) {
            ERROR("Allocate for partial read failed!");
            ret = AVB_IO_RESULT_ERROR_OOM;
            goto out;
        }

        LTRACEF("\nread from:0x%llx  size:0x%llx allocate size:0x%llx\n",
                read_offset, unaligned_num_bytes,
                round_up(unaligned_num_bytes, len_align_req));

        read_status = storage->read(storage, ptn + read_offset, block_buffer,
                                    round_up(unaligned_num_bytes, len_align_req));

        if (read_status) {
            ERROR("storage read error!\n");
            ret = AVB_IO_RESULT_ERROR_IO;
            goto out;
        }

        LTRACEF("\nmove from:xxx+0x%llx to 0x%p size:0x%llx \n",
                unaligned_offset, buffer, unaligned_num_bytes);
        /* Because unaligned_offset bytes invalid data has been read,
         * needs to skip these data.
         * */
        memcpy(buffer,
               block_buffer + unaligned_offset,
               unaligned_num_bytes - unaligned_offset);
        *out_num_read = unaligned_num_bytes - unaligned_offset;
    }

    ret = AVB_IO_RESULT_OK;
out:

    if (block_buffer != NULL) {
        free(block_buffer);
    }

    return ret;
}

static AvbIOResult avb_read_from_prtition(AvbOps *ops,
        const char *partition,
        int64_t offset, size_t num_bytes,
        void *buffer, size_t *out_num_read)
{
    AvbOpsUserData *user_data = NULL;
    struct image_load_info *img_info = NULL;

    if (!ops || !partition || !buffer || !out_num_read || num_bytes <= 0) {
        ERROR("bad input paramaters\n");
        return AVB_IO_RESULT_ERROR_IO;
    }

    user_data = ops->user_data;

    if (offset >= 0) {
        list_for_every_entry(&(user_data->preload_head), img_info,
                             struct image_load_info, node) {
            if (!strcmp(img_info->name, partition)
                    && ((uint64_t)offset + num_bytes <= img_info->size)) {
                memcpy(buffer, (void *)img_info->addr, num_bytes);
                *out_num_read = num_bytes;
                return AVB_IO_RESULT_OK;
            }
        }
    }

    return read_data_from_partition(user_data->ptdev, partition, offset,
                                    num_bytes, buffer, out_num_read);
}

static AvbIOResult avb_write_to_partition(AvbOps *ops,
        const char *partition,
        int64_t offset,
        size_t num_bytes, const void *buffer)
{
    /* unsupported api */
    return AVB_IO_RESULT_ERROR_IO;
}

static bool check_root_key_blob(struct public_key_blob *pk_blob)
{
    uint8_t digest[SHA256_DIGEST_LENGTH] __attribute__((aligned(
                CACHE_LINE))) = {0};
    uint32_t rotpk_hash[SHA256_DIGEST_LENGTH / 4] = {0};

    for (uint32_t i = 0; i < SHA256_DIGEST_LENGTH / 4; i++) {
        rotpk_hash[i] = fuse_read(ROTPK_FUSE_INDEX + i);
    }

    calc_hash(pk_blob->blob, pk_blob->blob_len, (uint8_t *)digest,
              SHA256_DIGEST_LENGTH, AVB_DIGEST_TYPE_SHA256);

    if (memcmp((uint8_t *)rotpk_hash, digest, SHA256_DIGEST_LENGTH)) {
        DEBUG_DUMP(pk_blob->blob, pk_blob->blob_len,
                   "root public key hash is not match!\n");

        if (!get_device_locked()) {
            ERROR("device is unlocked, ignore validity of root public key!\n");
            DEBUG_DUMP(digest, SHA256_DIGEST_LENGTH, "pk blob hash:\n");
            DEBUG_DUMP(rotpk_hash, SHA256_DIGEST_LENGTH, "saved rotpk hash:\n");
            return true;
        }

        return false;
    }

    return true;
}

uint32_t avb_get_image_size_from_bpt(const uint8_t *buffer, uint32_t len)
{
    uint32_t crc_val = 0;
    uint32_t crc_val_orig = 0;
    uint16_t iib_tag = 0;
    const uint8_t *iib = NULL;
    uint32_t img_sz = 0;

    if (len < BPT_SIZE || *(uint32_t *)buffer != BPT_TAG) {
        ERROR("invalid bpt size/tag!\n");
        goto out;
    }

    crc_val_orig = *((uint32_t *)(buffer + BPT_CHKSUM_OFFSET));
    crc_val = sd_crc32(0, buffer, BPT_CHKSUM_OFFSET);

    if (crc_val != crc_val_orig) {
        ERROR("BPT crc32 checksum error, orig:0x%0x calc:0x%0x!\n",
              crc_val_orig,
              crc_val);
        goto out;
    }

    iib = buffer + IIB_OFFSET;
    iib_tag = *(uint16_t *)iib;

    if (iib_tag != IIB_TAG) {
        ERROR("iib tag error:0x%0x!\n", iib_tag);
        goto out;
    }

    img_sz = *(uint32_t *)(iib + IMG_SZ_IIB_OFFSET);
out:
    return img_sz;
}

struct public_key_blob *avb_get_public_key_blob_from_bpt(
    const uint8_t *buffer, uint32_t len)
{
    uint32_t crc_val = 0;
    uint32_t blob_pad_size;
    uint32_t crc_val_orig = 0;
    uint16_t root_ca_tag = 0;
    const uint8_t *root_ca = NULL;
    struct public_key_blob *blob = NULL;

    if (len < BPT_SIZE || *(uint32_t *)buffer != BPT_TAG) {
        ERROR("invalid bpt size/tag!\n");
        goto fail;
    }

    crc_val_orig = *((uint32_t *)(buffer + BPT_CHKSUM_OFFSET));
    crc_val = sd_crc32(0, buffer, BPT_CHKSUM_OFFSET);

    if (crc_val != crc_val_orig) {
        ERROR("BPT crc32 checksum error, orig:0x%0x calc:0x%0x!\n",
              crc_val_orig,
              crc_val);
        goto fail;
    }

    root_ca = buffer + ROOT_CA_OFFSET;
    root_ca_tag = *(uint16_t *)root_ca;

    if (root_ca_tag != ROOT_CA_TAG) {
        ERROR("root cert tag error:0x%0x!\n", root_ca_tag);
        goto fail;
    }

    blob_pad_size = round_up(sizeof(struct public_key_blob), CACHE_LINE);
    blob = memalign(CACHE_LINE, blob_pad_size + ROOT_CA_SIZE);

    if (!blob) {
        ERROR("allocate memory for blob fail!\n");
        goto fail;
    }

    blob->blob = (uint8_t *)blob + blob_pad_size;
    blob->blob_len = ROOT_CA_SIZE;
    memcpy(blob->blob, root_ca, ROOT_CA_SIZE);

    return blob;
fail:

    if (!blob)
        free(blob);

    return NULL;
}

static bool verify_one_cert(const uint8_t *modulus, uint32_t modulus_len,
                            const uint8_t *expo, uint32_t expo_len,
                            X509_CERT *user_cert)
{
    int verify_ret = 0;
    X509_SIGN_T sign_type = SIGN_UNKNOWN_TYPE;

    if (!modulus || !user_cert || !expo || !modulus_len || !expo_len)
        return false;

    sign_type = user_cert->sign_type;

    if (user_cert->sign_type == SIGN_UNKNOWN_TYPE)
        return false;

    LTRACEF("cert sign type:%d\n", sign_type);
    verify_ret = verify_cert_sign(user_cert->tbs, user_cert->tbs_len,
                                  user_cert->sign, user_cert->sign_len,
                                  modulus, modulus_len,
                                  expo, expo_len,
                                  sign_type);

    if (verify_ret) {
        return false;
    }

    return true;
}

/* The last cert in the chain  is root CA */
static bool verify_cert_chain(X509_CERT *cert_chain,
                              uint32_t cert_count)
{
    X509_CERT *user_cert = NULL;
    X509_CERT *ca_cert   = NULL;
    uint32_t index = cert_count - 1;

    if (cert_count < 2 || !cert_chain) {
        ERROR("the number of certificates in chain must be larger than 2\n");
        return false;
    }

    ca_cert = &cert_chain[index];
    user_cert = &cert_chain[index];

    do {
        if (!verify_one_cert(ca_cert->n, ca_cert->n_len, ca_cert->e,
                             ca_cert->e_len,  user_cert)) {
            ERROR("verify certificate chain fail!\n");
            return false;
        }

        if (index == 0)
            break;

        ca_cert = &cert_chain[index];
        user_cert = &cert_chain[index - 1];

    }
    while (index-- >= 0) ;

    return true;
}

static struct public_key_blob *
make_pk_blob(X509_CERT *ca)
{
    uint8_t *pos = NULL;
    uint32_t blob_pad_size;
    struct public_key_blob *blob = NULL;

    if (!ca) {
        return NULL;
    }

    blob_pad_size = round_up(sizeof(struct public_key_blob), CACHE_LINE);
    blob = memalign(CACHE_LINE, blob_pad_size + ROOT_CA_SIZE);

    if (!blob) {
        ERROR("make public key blob fail, no memory!\n");
        return NULL;
    }

    memset(blob, 0x0, blob_pad_size + ROOT_CA_SIZE);
    blob->blob = (uint8_t *)blob + blob_pad_size;
    blob->blob_len = ROOT_CA_SIZE;

    pos = blob->blob;
    *(uint16_t * )pos = ROOT_CA_TAG;
    pos += sizeof(uint16_t);

    *(uint16_t * )pos = ROOT_CA_SIZE;
    pos += sizeof(uint16_t);

    *pos = ROOT_CA_SECURITY;
    pos++;

    if (ca->pubkey_type == PUBKEY_RSA) {
        if (ca->n_len == 128)
            *pos = PK_RSA_1024;
        else if (ca->n_len == 256)
            *pos = PK_RSA_2048;
        else if (ca->n_len == 384)
            *pos = PK_RSA_3072;
        else if (ca->n_len == 512)
            *pos = PK_RSA_4096;
        else {
            ERROR("unspport pubkey type:%d!\n", ca->pubkey_type);
            goto fail;
        }

        pos = blob->blob + PK_CA_OFFSET;
        *(uint32_t *)pos = ca->n_len;
        pos += sizeof(uint32_t);

        memcpy(pos, ca->n, ca->n_len);
        pos += PK_RSA_MAX_LEN;

        memcpy(pos + ca->n_len - ca->e_len, ca->e, ca->e_len);
    }
    else {
        if (ca->pubkey_type == PUBKEY_EC_P256V1) {
            *pos = PK_EC_P256V1;
        }
        else if (ca->pubkey_type == PUBKEY_EC_SECP384R1) {
            *pos = PK_EC_SECP384R1;
        }
        else if (ca->pubkey_type == PUBKEY_EC_SECP521R1) {
            *pos = PK_EC_SECP521R1;
        }
        else {
            ERROR("unspport pubkey type:%d!\n", ca->pubkey_type);
            goto fail;
        }

        pos = blob->blob + PK_CA_OFFSET;
        *(uint32_t *)pos = ca->n_len / 2;
        pos += sizeof(uint32_t);

        memcpy(pos, ca->n, ca->n_len / 2);
        pos += PK_EC_Q_MAX_LEN;

        memcpy(pos, ca->n + ca->n_len / 2, ca->n_len / 2);
    }

    return blob;
fail:

    if (blob)
        free(blob);

    return NULL;
}

static bool check_root_key_from_cert(X509_CERT *ca)
{
    bool ret = false;
    struct public_key_blob *blob = make_pk_blob(ca);

    if (!blob) {
        ERROR("make public key blob fail!\n");
        return false;
    }

    ret = check_root_key_blob(blob);
    free(blob);

    return ret;
}

static AvbIOResult avb_validate_vbmeta_publickey(AvbOps *ops,
        const uint8_t *public_key_data,
        size_t public_key_length,
        const uint8_t *public_key_metadata,
        size_t public_key_metadata_length,
        bool *out_is_trusted)
{
    uint32_t n_len = 0;
    int cert_parse_ret = 0;
    const uint8_t *n = NULL;
    X509_CERT *vbmeta_cert = NULL;
    X509_CERT *cert_chain = NULL;
    const uint8_t avb_pk_e[] = AVB_RSA_PK_E;
    AvbIOResult ret = AVB_IO_RESULT_OK;
    AvbRSAPublicKeyHeader avb_pubkey_header = {0};

    LTRACEF("ValidateVbmetaPublicKey PublicKeyLength %llu, "
            "PublicKeyMetadataLength %llu\n",
            (uint64_t)public_key_length, (uint64_t)public_key_metadata_length);

    if (ops == NULL || public_key_data == NULL || out_is_trusted == NULL) {
        ERROR("Invalid parameters\n");
        ret = AVB_IO_RESULT_ERROR_IO;
        goto out;
    }

    *out_is_trusted = false;

    if (!avb_rsa_public_key_header_validate_and_byteswap(
                (const AvbRSAPublicKeyHeader *)public_key_data, &avb_pubkey_header)) {
        ERROR("Invalid key.\n");
        ret = AVB_IO_RESULT_ERROR_IO;
        goto out;
    }

    cert_parse_ret = X509CertChainParse(public_key_metadata,
                                        public_key_metadata_length, &cert_chain);

    if (cert_parse_ret > 1) {
        LTRACEF("find certificate chain\n");

        if (!check_root_key_from_cert(&cert_chain[cert_parse_ret - 1])) {
            ERROR("check root key from certificate fail!\n");
            goto out;
        }

        if (!verify_cert_chain(cert_chain, cert_parse_ret)) {
            ERROR("check certificate chain fail!\n");
            ret = AVB_IO_RESULT_ERROR_IO;
            goto out;
        }
    }
    else {
        ERROR("There is no certificate in vbmeta partition\n");
        ret = AVB_IO_RESULT_ERROR_IO;
        goto out;

    }

    n_len = avb_pubkey_header.key_num_bits / 8;
    n = public_key_data + sizeof(AvbRSAPublicKeyHeader);
    vbmeta_cert = &cert_chain[0];

    if (n_len != vbmeta_cert->n_len
            || sizeof(avb_pk_e) != vbmeta_cert->e_len
            || memcmp(vbmeta_cert->n, n, n_len)
            || memcmp(vbmeta_cert->e, avb_pk_e, sizeof(avb_pk_e))) {
        ERROR("modulus in publick key_data isn't equal to the one in public_key_metadata\n");
        ret = AVB_IO_RESULT_ERROR_IO;
        goto out;
    }

    ret = AVB_IO_RESULT_OK;
    *out_is_trusted = true;
out:

    if (cert_chain)
        X509CertChainFree(cert_chain, cert_parse_ret);

    return ret;
}

static AvbIOResult avb_read_rollback_index(AvbOps *ops,
        size_t rollback_index_location,
        uint64_t *out_rollback_index)
{
    uint32_t tmv  = 0U;
    uint32_t loop = 0U;
    uint32_t aligned_bytes = 0U;
    uint32_t aligned_req   = 0U;
    uint64_t rollback_index = 0U;
    uint32_t unaligned_bytes = 0U;
    uint32_t unaligned_mask  = 0U;

    /* write/read  a word fuse value at a time */
    aligned_req = sizeof(uint32_t);
    aligned_bytes = round_up(RMV_FUSE_SIZE_BYTE, aligned_req);
    loop = aligned_bytes / aligned_req - 1;

    /* RMV_FUSE_SIZE_BYTE may be not aligned to unsigned int */
    unaligned_bytes = aligned_bytes - RMV_FUSE_SIZE_BYTE;
    unaligned_mask  = ~0U >> (unaligned_bytes * 8);
    LTRACEF("unaligned_mask:0x%0x\n", unaligned_mask);

    /* detect from the most significant bit */
    tmv = fuse_read(RMV_FUSE_INDEX + loop);
    tmv &= unaligned_mask;
    LTRACEF("loop:%u\n", loop);

    while (tmv == 0 && loop) {
        tmv = 0;
        fuse_read(RMV_FUSE_INDEX + (--loop));
        LTRACEF("loop:%u\n", loop);
    }

    if (tmv) {
        rollback_index = aligned_req * 8 - __builtin_clz(tmv);
        rollback_index += aligned_req * loop * 8;
        LTRACEF("rollback_index:%llu loop:%u\n", rollback_index, loop);
    }

    *out_rollback_index = rollback_index;
    LTRACEF("rollback_index_location:%llu  rollback_index:%llu\n",
            (uint64_t)rollback_index_location, (uint64_t)rollback_index);
    return AVB_IO_RESULT_OK;
}

static AvbIOResult avb_write_rollback_index(AvbOps *ops,
        size_t rollback_index_location,
        uint64_t rollback_index)
{
    AvbIOResult ret = AVB_IO_RESULT_OK;
    uint32_t tmv  = 0U;
    uint32_t mask = 0U;
    uint32_t bank_index  = 0;
    uint32_t bit_in_bank = 0;
    uint32_t aligned_bits = 0U;
    uint32_t aligned_bit_req   = 0U;
    uint64_t rollback_index_stored = 0;
    partition_device_t  *ptdev = NULL;
    AvbOpsUserData *user_data = NULL;

    user_data = ops->user_data;
    ptdev = user_data->ptdev;

    if (!ptdev) {
        ERROR("partition device is null.\n");
        return AVB_IO_RESULT_OK;
    }

    if ( rollback_index_location != 0 ) {
        ERROR("rollback_index_location must be 0.\n");
        return AVB_IO_RESULT_ERROR_IO;
    }

    ret =  avb_read_rollback_index(ops, rollback_index_location,
                                   &rollback_index_stored);

    if (ret != AVB_IO_RESULT_OK) {
        ERROR("read rollback_index_stored fail\n");
        return ret;
    }

    if (rollback_index_stored > rollback_index) {
        ERROR("rollback_index is smaller than stored rollback index.\n");
        return AVB_IO_RESULT_ERROR_IO;
    }

    if (rollback_index > RMV_FUSE_SIZE_BYTE * 8) {
        ERROR("rollback_index too large.\n");
        return AVB_IO_RESULT_ERROR_IO;
    }

    if (rollback_index_stored == rollback_index)
        return AVB_IO_RESULT_OK;

    if (ptdev->multislot_support && !is_current_slot_successful(ptdev)) {
        ERROR(" No update rollback index, current slot is unbootable\n");
        return AVB_IO_RESULT_OK;
    }

    /* write/read  a word fuse value at a time */
    aligned_bit_req = sizeof(uint32_t) * 8;
    mask = 1U << (aligned_bit_req - 1);
    aligned_bits = round_up(rollback_index, aligned_bit_req);
    bank_index = aligned_bits / (aligned_bit_req) - 1;

    tmv = fuse_read(RMV_FUSE_INDEX + bank_index);
    bit_in_bank = aligned_bits - rollback_index;
    tmv |= mask >> bit_in_bank;
    LTRACEF("tmv:0x%0x bank_index:%u!\n", tmv, bank_index);

#if 0

    if (fuse_program(RMV_FUSE_INDEX + bank_index, tmv)) {
        ERROR("write rollback index fail!\n");
        return AVB_IO_RESULT_ERROR_IO;
    }

#endif
    LTRACEF("write rollback index:%llu successfully!\n", rollback_index);
    return AVB_IO_RESULT_OK;
}

bool get_device_locked(void)
{
    uint32_t val = 0;

    val = fuse_read(PROD_FUSE_INDEX);

    return !!(val & 1U << PROD_FUSE_BIT);
}

static AvbIOResult avb_read_is_device_unlocked(AvbOps *ops,
        bool *out_is_unlocked)
{
    if (out_is_unlocked == NULL) {
        ERROR("bad input paramaters\n");
        return AVB_IO_RESULT_ERROR_IO;
    }

    *out_is_unlocked = !get_device_locked();
    return AVB_IO_RESULT_OK;
}

static void guid2hex(char *buf, EFI_GUID *guid)
{
    uint8_t hex_digits[17] = "0123456789abcdef";

    buf[0] = hex_digits[(guid->data1 >> 28) & 0x0f];
    buf[1] = hex_digits[(guid->data1 >> 24) & 0x0f];
    buf[2] = hex_digits[(guid->data1 >> 20) & 0x0f];
    buf[3] = hex_digits[(guid->data1 >> 16) & 0x0f];
    buf[4] = hex_digits[(guid->data1 >> 12) & 0x0f];
    buf[5] = hex_digits[(guid->data1 >> 8) & 0x0f];
    buf[6] = hex_digits[(guid->data1 >> 4) & 0x0f];
    buf[7] = hex_digits[(guid->data1 >> 0) & 0x0f];
    buf[8] = '-';
    buf[9] = hex_digits[(guid->data2 >> 12) & 0x0f];
    buf[10] = hex_digits[(guid->data2 >> 8) & 0x0f];
    buf[11] = hex_digits[(guid->data2 >> 4) & 0x0f];
    buf[12] = hex_digits[(guid->data2 >> 0) & 0x0f];
    buf[13] = '-';
    buf[14] = hex_digits[(guid->data3 >> 12) & 0x0f];
    buf[15] = hex_digits[(guid->data3 >> 8) & 0x0f];
    buf[16] = hex_digits[(guid->data3 >> 4) & 0x0f];
    buf[17] = hex_digits[(guid->data3 >> 0) & 0x0f];
    buf[18] = '-';
    buf[19] = hex_digits[(guid->data4[0] >> 4) & 0x0f];
    buf[20] = hex_digits[(guid->data4[0] >> 0) & 0x0f];
    buf[21] = hex_digits[(guid->data4[1] >> 4) & 0x0f];
    buf[22] = hex_digits[(guid->data4[1] >> 0) & 0x0f];
    buf[23] = '-';
    buf[24] = hex_digits[(guid->data4[2] >> 4) & 0x0f];
    buf[25] = hex_digits[(guid->data4[2] >> 0) & 0x0f];
    buf[26] = hex_digits[(guid->data4[3] >> 4) & 0x0f];
    buf[27] = hex_digits[(guid->data4[3] >> 0) & 0x0f];
    buf[28] = hex_digits[(guid->data4[4] >> 4) & 0x0f];
    buf[29] = hex_digits[(guid->data4[4] >> 0) & 0x0f];
    buf[30] = hex_digits[(guid->data4[5] >> 4) & 0x0f];
    buf[31] = hex_digits[(guid->data4[5] >> 0) & 0x0f];
    buf[32] = hex_digits[(guid->data4[6] >> 4) & 0x0f];
    buf[33] = hex_digits[(guid->data4[6] >> 0) & 0x0f];
    buf[34] = hex_digits[(guid->data4[7] >> 4) & 0x0f];
    buf[35] = hex_digits[(guid->data4[7] >> 0) & 0x0f];
    buf[36] = '\0';
}

static AvbIOResult avb_get_unique_guid_for_partition(AvbOps *ops,
        const char *partition,
        char *guid_buf,
        size_t guid_buf_size)
{
    AvbOpsUserData *user_data = NULL;
    partition_device_t  *ptdev = NULL;
    struct partition_entry *pt_entry = NULL;
    uint32_t pt_index = 0;
    uint8_t unique_partition_guid[UNIQUE_PARTITION_GUID_SIZE];

    user_data = ops->user_data;
    ptdev     = user_data->ptdev;

    /* when download mode, vbmeta image appended to dloader/ospiprog,
     * and there is no such partition in storage, so use default uuid
     * */
    if (!ptdev && !strcmp(partition, VBMETA_PARTITION_NAME)) {
        strncpy(guid_buf, DEFAULT_VBMETA_UUID, sizeof(DEFAULT_VBMETA_UUID));
        return AVB_IO_RESULT_OK;

    }

    pt_index  = ptdev_get_index(ptdev, partition);
    pt_entry  = ptdev_get_partition_entries(ptdev);

    LTRACEF("get partition uuid:%s\n", partition);

    if (!pt_entry || pt_index == (uint32_t)INVALID_PTN) {
        ERROR("get_unique_guid: No partition entry for %s\n",
              partition);
        return AVB_IO_RESULT_ERROR_IO;
    }

    memcpy(unique_partition_guid,
           pt_entry[pt_index].unique_partition_guid,
           UNIQUE_PARTITION_GUID_SIZE);

    if ((strlen(partition) + 1) > MAX_GPT_NAME_SIZE) {
        ERROR("avb_get_unique_guid_for_partition: Partition "
              "%s, name too large\n", partition);
        return AVB_IO_RESULT_ERROR_IO;
    }

    guid2hex(guid_buf, (EFI_GUID *)unique_partition_guid);
    LTRACEF("%s uuid: %s\n", partition, guid_buf);

    return AVB_IO_RESULT_OK;
}

static AvbIOResult avb_get_size_of_partition(AvbOps *ops,
        const char *partition,
        uint64_t *out_size_num_bytes)
{
    AvbOpsUserData *user_data = NULL;
    partition_device_t  *ptdev = NULL;
    struct image_load_info *img_info = NULL;

    if (ops == NULL || partition == NULL || out_size_num_bytes == NULL) {
        ERROR("avb_get_size_of_partition invalid parameter pointers\n");
        return AVB_IO_RESULT_ERROR_IO;
    }

    user_data = ops->user_data;
    ptdev = user_data->ptdev;

    list_for_every_entry(&(user_data->preload_head), img_info,
                         struct image_load_info, node) {
        if (!strcmp(img_info->name, partition)) {
            *out_size_num_bytes = img_info->size;
            LTRACEF("preload partition size:%llu\n", *out_size_num_bytes);
            return AVB_IO_RESULT_OK;
        }
    }

    if (ptdev) {
        *out_size_num_bytes = ptdev_get_size(ptdev, partition);
    }
    else {
        return AVB_IO_RESULT_ERROR_IO;
    }

    return AVB_IO_RESULT_OK;
}

AvbOps *avb_ops_new(partition_device_t  *ptdev,
                    struct public_key_blob *pk_blob)
{
    uint32_t len = 0;
    AvbOps *ops = NULL;
    AvbOpsUserData *user_data = NULL;

    len = sizeof(AvbOps) + sizeof(AvbOpsUserData);
    ops = calloc(1, len);

    if (ops == NULL) {
        ERROR("Error allocating memory for AvbOps.\n");
        goto fail;
    }

    user_data = (AvbOpsUserData *)(ops + 1);
    user_data->ptdev = ptdev;

    if (pk_blob) {
        user_data->pk_blob = pk_blob;
        user_data->pk_blob_preloaded = true;
    }

    list_initialize(&user_data->preload_head);

    ops->user_data = user_data;
    ops->get_preloaded_partition       = avb_get_preload_partition;
    ops->read_from_partition           = avb_read_from_prtition;
    ops->write_to_partition            = avb_write_to_partition;
    ops->validate_vbmeta_public_key    = avb_validate_vbmeta_publickey;
    ops->read_rollback_index           = avb_read_rollback_index;
    ops->write_rollback_index          = avb_write_rollback_index;
    ops->read_is_device_unlocked       = avb_read_is_device_unlocked;
    ops->get_unique_guid_for_partition = avb_get_unique_guid_for_partition;
    ops->get_size_of_partition         = avb_get_size_of_partition;

    crypto_eng_init();
    return ops;
fail:

    if (ops)
        free(ops);

    return NULL;
}

void avb_ops_free(AvbOps *ops)
{
    struct image_load_info *img_info = NULL;
    struct AvbOpsUserData *user_data = NULL;

    if (!ops)
        goto out;

    user_data = ops->user_data;

    while (1) {
        img_info = list_remove_tail_type(&user_data->preload_head,
                                         struct image_load_info, node);

        if (!img_info)
            break;

        image_info_node_free(img_info);
    }

    if (user_data->pk_blob && !user_data->pk_blob_preloaded)
        free(user_data->pk_blob);

    crypto_eng_deinit();
out:

    if (ops)
        free(ops);

    return;
}

struct image_load_info *image_info_node_new(addr_t addr, size_t size,
        const char *name)
{
    struct image_load_info *img_info = NULL;

    img_info = calloc(1, sizeof(struct image_load_info));

    if (!img_info) {
        ERROR("allocate memory error!");
        goto fail;
    }

    img_info->addr = addr;
    img_info->size = size;
    img_info->name = strdup(name);

    if (!img_info->name) {
        ERROR("allocate memory error!");
        goto fail;
    }

    list_initialize(&img_info->node);
    return img_info;
fail:

    if (img_info)
        free(img_info);

    return NULL;
}

void image_info_node_free(struct image_load_info *img_info)
{
    addr_t name = 0;

    if (img_info) {
        name = (addr_t)img_info->name;

        if (img_info->name)
            free((void *)name);

        free(img_info);
    }

    return;
}

bool avb_add_preload_image_info(AvbOps *ops, addr_t addr,
                                size_t size,
                                const char *name)
{
    AvbOpsUserData *user_data = NULL;
    struct image_load_info *img_info = NULL;

    LTRACEF(" parition:%s\n", name);

    if (!ops) {
        ERROR("ops is null!");
        return false;
    }

    user_data = ops->user_data;

    img_info = image_info_node_new(addr, size, name);

    if (!img_info)
        return false;

    list_add_tail(&user_data->preload_head, &img_info->node);
    return true;
}


bool avb_get_footer_from_buffer(const uint8_t *buf, uint32_t buf_len,
                                AvbFooter *out_footer)
{
    uint32_t pos = 0;

    if (buf_len < AVB_FOOTER_SIZE) {
        ERROR("%s buf_len:%u error!\n", __func__, buf_len);
        return false;
    }

    pos = buf_len - AVB_FOOTER_SIZE;

    if (!avb_footer_validate_and_byteswap((const AvbFooter *)(buf + pos),
                                          out_footer)) {
        avb_debugv(full_partition_name, ": No footer detected.\n", NULL);
        return false;
    }

    return true;
}

bool avb_get_footer_from_partition(partition_device_t  *ptdev,
                                   const char *partition, AvbFooter *out_footer)
{
    AvbFooter footer = {0};
    bool ret = false;
    size_t out_num_read = 0;
    uint8_t *footer_buf = NULL;

    if (!ptdev) {
        ERROR("ptdevi is null\n");
        goto out;
    }

    footer_buf = calloc(1, AVB_FOOTER_SIZE);

    if (!footer_buf) {
        ERROR("allocate memory fail\n");
        goto out;
    }

    ret = read_data_from_partition(ptdev, partition, -AVB_FOOTER_SIZE,
                                   AVB_FOOTER_SIZE, footer_buf, &out_num_read);

    if (ret != AVB_IO_RESULT_OK || out_num_read != AVB_FOOTER_SIZE) {
        ERROR("read footer of partition %s error!\n", partition);
        goto out;
    }

    if (!avb_footer_validate_and_byteswap((const AvbFooter *)footer_buf,
                                          &footer)) {
        avb_debugv(full_partition_name, ": No footer detected.\n", NULL);
    }

    if (out_footer)
        memcpy(out_footer, &footer, AVB_FOOTER_SIZE);

    ret = true;
out:

    if (footer_buf)
        free(footer_buf);

    return ret;
}

bool avb_get_vbmeta_header_from_buffer(const uint8_t *buffer,
                                       uint32_t buf_len, AvbVBMetaImageHeader *vbmeta)
{
    if (!buffer || buf_len < AVB_VBMETA_IMAGE_HEADER_SIZE) {
        ERROR("%s buffer error!\n", __func__);
        return false;
    }

    avb_vbmeta_image_header_to_host_byte_order((AvbVBMetaImageHeader *)buffer,
            vbmeta);
    return true;
}

bool verify_single_image(partition_device_t  *ptdev, uint8_t *buffer,
                         uint32_t buf_len, const char *partition, AvbSlotVerifyData **out_slot_data)
{
    struct list_node head;
    struct image_load_info img_info = {0};

    img_info.addr = (addr_t)buffer;
    img_info.size = buf_len;
    img_info.name = partition;
    list_initialize(&head);
    list_add_tail(&head, &img_info.node);

    return verify_loaded_images(ptdev, &head, out_slot_data);
}

bool verify_loaded_images(partition_device_t  *ptdev,
                          struct list_node *head, AvbSlotVerifyData **out_slot_data)
{
    bool ret = false;
    bool verified = false;
    uint32_t part_count = 0;
    AvbOps *avb_ops = NULL;
    const char **partition  = NULL;
    const char **request_partition  = NULL;
    struct image_load_info *img_info = NULL;
    AvbSlotVerifyData *slot_data = NULL;
    AvbSlotVerifyResult verify_ret    = AVB_SLOT_VERIFY_RESULT_OK;
    AvbSlotVerifyFlags verify_flags   = AVB_SLOT_VERIFY_FLAGS_NONE;
    AvbHashtreeErrorMode verity_flags = AVB_HASHTREE_ERROR_MODE_EIO;

    if (out_slot_data)
        *out_slot_data = NULL;

    part_count = list_length(head);
    request_partition = calloc(1, (part_count + 1) * sizeof(const char **));

    if (!request_partition) {
        ERROR("allocate memory error!");
        goto out;
    }

    avb_ops = avb_ops_new(ptdev, NULL);

    if (!avb_ops) {
        ERROR("ops allocate memory error!");
        goto out;
    }

    partition = request_partition;
    list_for_every_entry(head, img_info, struct image_load_info, node) {

        avb_add_preload_image_info(avb_ops, img_info->addr, img_info->size,
                                   img_info->name);
        *partition++ = img_info->name;
    }

    if (!get_device_locked()) {
        verify_flags = AVB_SLOT_VERIFY_FLAGS_ALLOW_VERIFICATION_ERROR;
        ERROR("device is unlocked, allow verification error!\n");
    }

    verify_ret = avb_slot_verify(avb_ops, request_partition, "\0",
                                 verify_flags, verity_flags, &slot_data);
    LTRACEF("verify ret:%s\n!",
            avb_slot_verify_result_to_string(verify_ret));

    if (verify_ret != AVB_SLOT_VERIFY_RESULT_OK
            && verify_flags != AVB_SLOT_VERIFY_FLAGS_ALLOW_VERIFICATION_ERROR) {
        ERROR("verify partition fail!\n");
        goto out;
    }

    LTRACEF("num vbmeta:%llu num partition:%llu cmdline:%s\n",
            (uint64_t)slot_data->num_vbmeta_images,
            (uint64_t)slot_data->num_loaded_partitions, slot_data->cmdline);

    list_for_every_entry(head, img_info, struct image_load_info, node) {
        verified = false;

        for (uint32_t i = 0; slot_data && i < slot_data->num_loaded_partitions; i++) {
            AvbPartitionData *loaded_partitions = &slot_data->loaded_partitions[i];

            if (!strcmp(img_info->name, loaded_partitions->partition_name)) {
                verified = true;
                LTRACEF("partition %s verified ok\n", img_info->name);
                break;
            }
        }

        if (!verified && strcmp(img_info->name, VBMETA_PARTITION_NAME)) {
            ERROR("partition %s is not verified!\n", img_info->name);
            goto out;
        }
    }

    if (out_slot_data)
        *out_slot_data = slot_data;

    ret = true;
out:

    if (slot_data && (!out_slot_data || !*out_slot_data))
        avb_slot_verify_data_free(slot_data);

    if (request_partition) {
        free(request_partition);
    }

    if (avb_ops)
        avb_ops_free(avb_ops);

    return ret;
}

bool add_verified_image_list(struct list_node *head, void *buf,
                             size_t size, const char *name)
{
    struct image_load_info *image_info;

    if (!head || !(image_info = image_info_node_new((addr_t)buf, size,
                                name))) {
        return false;
    }

    LTRACEF("add partition %s to verify\n", name);
    list_add_tail(head, &image_info->node);
    return true;
}

void free_image_info_list(struct list_node *head)
{
    struct image_load_info *img_info;

    if (!head)
        return;

    list_for_every_entry(head, img_info, struct image_load_info, node) {
        image_info_node_free(img_info);
    }
}
