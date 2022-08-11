#ifndef _PLAT_INTERRUPT_H_
#define _PLAT_INTERRUPT_H_

#ifndef  MAX_INTERRUPT_NUM
#define MAX_INTERRUPT_NUM   298
#endif
#ifdef SHELL_USE_USB
typedef struct {
    void (*hdlr)(void);
} interrupt_hdlr_t;

void plat_interrupt_hdlr(void);
void plat_interrupt_init(void);
void plat_setup_interrupt(uint32_t id, uint32_t pri, void (*f)(void));
void plat_enable_interrupt(uint32_t id);
void plat_disable_interrupt(uint32_t id);
#endif
#endif  /* _PLAT_INTERRUPT_H_ */
