#include "__regs_base.h"

#define GIC_BASE GIC1_BASE
#if WITH_KERNEL_VM
	#define GICBASE(n)  (GIC_BASE + PERIPHERAL_BASE_VIRT)
#else
	#define GICBASE(n)  (GIC1_BASE)
#endif
#define GICD_OFFSET (0x1000)
#define GICC_OFFSET (0x2000)
//#define GICC_OFFSET (0x4000)
#define MAX_INT (1023)
