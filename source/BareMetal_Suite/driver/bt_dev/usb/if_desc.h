#ifndef __IF_DESC_H__
#define __IF_DESC_H__

#define FS_IF_DESC   \
        0x09,           /* length */\
        INTERFACE,      /* type */\
        0,              /* interface num */\
        0x00,           /* alternates */\
        0x02,           /* endpoint count */\
        0xff,           /* interface class */\
        0x42,           /* interface subclass */\
        0x03,           /* interface protocol */\
        0x00,           /* string index */\
        /* endpoint 1 IN */\
        0x07,           /* length */\
        ENDPOINT,       /* type */\
        0x1 | 0x80,    /* address: 1 IN */\
        0x02,           /* type: bulk */\
        W(64),          /* max packet size: 64 */\
        00,             /* interval */\
        /* endpoint 1 OUT */\
        0x07,           /* length */\
        ENDPOINT,       /* type */\
        0x1,            /* address: 1 OUT */\
        0x02,           /* type: bulk */\
        W(64),          /* max packet size: 64 */\
        00,             /* interval */\

#define FS_IF_DESC_LEN      (9 + 7 + 7)

#define HS_IF_DESC   \
        0x09,           /* length */\
        INTERFACE,      /* type */\
        0,              /* interface num */\
        0x00,           /* alternates */\
        0x02,           /* endpoint count */\
        0xff,           /* interface class */\
        0x42,           /* interface subclass */\
        0x03,           /* interface protocol */\
        0x00,           /* string index */\
        /* endpoint 1 IN */\
        0x07,           /* length */\
        ENDPOINT,       /* type */\
        1 | 0x80,    /* address: 1 IN */\
        0x02,           /* type: bulk */\
        W(512),          /* max packet size: 64 */\
        00,             /* interval */\
        /* endpoint 1 OUT */\
        0x07,           /* length */\
        ENDPOINT,       /* type */\
        1,          /* address: 1 OUT */\
        0x02,           /* type: bulk */\
        W(512),          /* max packet size: 64 */\
        00,             /* interval */\

#define HS_IF_DESC_LEN      (9 + 7 + 7)

#endif
