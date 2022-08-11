#ifndef __SURFACEINFO_PUBLIC_H
#define __SURFACEINFO_PUBLIC_H
#include <stdint.h>
namespace sdm {

struct Rect {
    int left;
    int top;
    int right;
    int bottom;
}__attribute__((__packed__));

struct Surface {    
    uint64_t phy_addr;
    int width;
    int height;
    int stride;
    size_t size;
    Rect source;
    Rect display;
    int format;
    int prime_fd;
    int cmd;
}__attribute__((__packed__));


enum {
    MSG_NOTIFY_SURFACE_INFO = 0,
	MSG_NOTIFY_HAS_CONTENT = 1,
	MSG_NOTIFY_NO_CONTENT,

	MSG_NOTIFY_MAX = 16,
};

}
#endif