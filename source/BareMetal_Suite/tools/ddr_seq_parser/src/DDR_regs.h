#ifndef __DDR_REGS_H__
#define __DDR_REGS_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t v;
    const char *name;
} reg_desc_t;

bool get_val_by_name(char *name, uint32_t *v);

#endif  /* __DDR_REGS_H__ */
