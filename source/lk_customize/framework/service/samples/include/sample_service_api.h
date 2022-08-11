#ifndef _SSP_API_H_
#define _SSP_API_H_

/*
 * let these definition compatiable with another OS side
 * Here define the io request and reply operation code
 */
enum sample_op_type {
    SSP_OP_GET_VERSION,
    SSP_OP_GET_CONFIG,
    SSP_OP_OPEN,
    SSP_OP_CLOSE,
    SSP_OP_START,
    SSP_OP_STOP,
    SSP_OP_MAX,
};

/* Do not exceed 32 bytes limitation */
struct sample_request {
    u16 op;
    u16 instance;
    union {
        struct {
            u16 what;
            u16 why;
            u32 how;
        } s;
    } msg;
};

/* Do not exceed 16 bytes limitation */
struct sample_reply {
    u16 op;
    u16 instance;
    union {
        /** used for get_version */
        struct {
            u16 version;
            u16 capability;
            u16 vendor;
        } v;
        u8 data[12];
    } msg;
};

/*
 * Sample service standard state
 */
enum sample_state_type {
    SSP_ST_INVALID,
    SSP_ST_IDLE,
    SSP_ST_STOPPED,
    SSP_ST_RUNNING,
    SSP_ST_STOPPING,
    SSP_ST_MAX,
};

/*
 * Sample payload with user data
 * user can customize payload structure
 * the payload format should be aligned with client side
 */
struct payload_t {
    uint16_t magic;
    uint16_t num;
    uint32_t size;
    char data[];
} __attribute__((__packed__));


#endif //_SSP_API_H_
