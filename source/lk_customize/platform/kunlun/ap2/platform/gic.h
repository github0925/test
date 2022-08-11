#include "__regs_base.h"
#include "target_res.h"

#define GIC_BASE GIC5_BASE
#if WITH_KERNEL_VM
	#define GICBASE(n)  (GIC_BASE + PERIPHERAL_BASE_VIRT)
#else
	#define GICBASE(n)  (GIC2_BASE)
#endif
#define GICD_OFFSET (0x1000)
#define GICC_OFFSET (0x2000)

#define MAX_INT         MAX_INT_NUM
//#define GICC_OFFSET (0x4000)
