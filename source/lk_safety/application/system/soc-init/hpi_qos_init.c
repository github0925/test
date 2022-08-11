/*
 * Copyright (c) 2020 Semidrive Inc.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <bits.h>
#include <stdio.h>

#include "__regs_base.h"
#include "chip_res.h"
#include "lib/reg.h"

/* HPI QOS config */
void init_qos_by_module(addr_t iobase, int offset,
                        uint32_t pri, uint32_t mode,
                        uint32_t bw, uint32_t satur, uint32_t ext)
{
    /* priority */
    writel(pri, iobase + offset + 0x8);
    /* mode */
    writel(mode, iobase + offset + 0xc);
    /* bw */
    writel(bw, iobase + offset + 0x10);
    /* saturation */
    writel(satur, iobase + offset + 0x14);
    /* ext */
    writel(ext, iobase + offset + 0x18);
}

void hpi_qos_init(void)
{
    addr_t iobase = 0;

    iobase = p2v(APB_GPV_DISP_BASE);
    /* G2D1 */
    init_qos_by_module(iobase, 0x180, 0x80000503, 0x00000003, 0x0000016c,
                       0x00000080, 0x0);
    /* G2D2 */
    init_qos_by_module(iobase, 0x200, 0x80000606, 0x00000003, 0x0000016c,
                       0x00000080, 0x0);

#if 1
    /* regulator mode */
    /* CSI1 */
    init_qos_by_module(iobase, 0, 0x80000503, 0x00000003, 0x00000111, 0x00000080,
                       0x0);
    /* CSI2 */
    init_qos_by_module(iobase, 0x80, 0x80000503, 0x00000003, 0x00000111, 0x00000080,
                       0x0);
    /* CSI3 */
    init_qos_by_module(iobase, 0x100, 0x80000503, 0x00000003, 0x00000111,
                       0x00000080, 0x0);
    /* DP1 */
    init_qos_by_module(iobase, 0x280, 0x80000704, 0x00000003, 0x000002d9,
                       0x00000200, 0x0);
    /* DC1 */
    init_qos_by_module(iobase, 0x300, 0x80000704, 0x00000003, 0x000002d9,
                       0x00000080, 0x0);
    /* DP2 */
    init_qos_by_module(iobase, 0x380, 0x80000704, 0x00000003, 0x000002d9,
                       0x00000200, 0x0);
    /* DC2 */
    init_qos_by_module(iobase, 0x400, 0x80000704, 0x00000003, 0x000002d9,
                       0x00000080, 0x0);
    /* DP3 */
    init_qos_by_module(iobase, 0x480, 0x80000704, 0x00000003, 0x000002d9,
                       0x00000200, 0x0);
    /* DC3 */
    init_qos_by_module(iobase, 0x500, 0x80000704, 0x00000003, 0x000002d9,
                       0x00000080, 0x0);
    /* DC4 */
    init_qos_by_module(iobase, 0x580, 0x80000704, 0x00000003, 0x0000016c,
                       0x00000080, 0x0);
    /* DC5 */
    init_qos_by_module(iobase, 0x600, 0x80000704, 0x00000003, 0x0000016c,
                       0x00000080, 0x0);
#else
    /* bypass mode */
    /* CSI1 */
    init_qos_by_module(iobase, 0, 0x80000503, 0x00000002, 0x0000016c, 0x00000080,
                       0x0);
    /* CSI2 */
    init_qos_by_module(iobase, 0x80, 0x80000503, 0x00000002, 0x0000005b, 0x00000080,
                       0x0);
    /* CSI3 */
    init_qos_by_module(iobase, 0x100, 0x80000503, 0x00000002, 0x0000005b,
                       0x00000080, 0x0);
    /* DP1 */
    init_qos_by_module(iobase, 0x280, 0x80000503, 0x00000002, 0x00000390,
                       0x00000200, 0x0);
    /* DC1 */
    init_qos_by_module(iobase, 0x300, 0x80000503, 0x00000002, 0x000000B6,
                       0x00000100, 0x0);
    /* DP2 */
    init_qos_by_module(iobase, 0x380, 0x80000503, 0x00000002, 0x00000390,
                       0x00000200, 0x0);
    /* DC2 */
    init_qos_by_module(iobase, 0x400, 0x80000503, 0x00000002, 0x000000b6,
                       0x00000100, 0x0);
    /* DP3 */
    init_qos_by_module(iobase, 0x480, 0x80000503, 0x00000002, 0x000001c8,
                       0x00000200, 0x0);
    /* DC3 */
    init_qos_by_module(iobase, 0x500, 0x80000503, 0x00000002, 0x0000005b,
                       0x00000100, 0x0);
    /* DC4 */
    init_qos_by_module(iobase, 0x580, 0x80000503, 0x00000002, 0x00000049,
                       0x00000100, 0x0);
    /* DC5 */
    init_qos_by_module(iobase, 0x600, 0x80000503, 0x00000002, 0x00000049,
                       0x00000100, 0x0);
#endif

#if 1
    iobase = p2v(APB_GPV_HIS_BASE);
    /* PCIE1 */
    init_qos_by_module(iobase, 0, 0x80000301, 0x00000003, 0x0000020a, 0x00000040,
                       0x0);
    /* PCIE2 */
    init_qos_by_module(iobase, 0x80, 0x80000402, 0x00000003, 0x0000020a, 0x00000040,
                       0x0);
    /* USB1 */
    init_qos_by_module(iobase, 0x100, 0x80000402, 0x00000003, 0x000001eb,
                       0x00000040, 0x0);
    /* USB2 */
    init_qos_by_module(iobase, 0x180, 0x80000402, 0x00000003, 0x000001eb,
                       0x00000040, 0x0);
#endif

    iobase = p2v(APB_GPV_VPU_BASE);
    /* vpu1.prim */
    init_qos_by_module(iobase, 0, 0x80000301, 0x00000003, 0x00000120, 0x00000040,
                       0x0);
    /* vpu1.2nd */
    init_qos_by_module(iobase, 0x80, 0x80000301, 0x00000003, 0x00000000, 0x00000040,
                       0x0);
    /* vpu1.proc */
    init_qos_by_module(iobase, 0x100, 0x80000301, 0x00000003, 0x00000030,
                       0x00000040, 0x0);
    /* vpu1.dma */
    init_qos_by_module(iobase, 0x180, 0x80000301, 0x00000003, 0x00000090,
                       0x00000040, 0x0);
    /* vpu1.sdma */
    init_qos_by_module(iobase, 0x200, 0x80000301, 0x00000003, 0x00000090,
                       0x00000040, 0x0);
    /* vpu2.prim */
    init_qos_by_module(iobase, 0x280, 0x80000301, 0x00000003, 0x00000090,
                       0x00000040, 0x0);
    /* vpu2.2nd */
    init_qos_by_module(iobase, 0x300, 0x80000301, 0x00000003, 0x00000000,
                       0x00000040, 0x0);
    /* MJPEG */
    init_qos_by_module(iobase, 0x380, 0x80000301, 0x00000003, 0x000000d8,
                       0x00000040, 0x0);

#if 0
    iobase = p2v(GPV_VSN_BASE);
    /* VDSP.prim */
    init_qos_by_module(iobase, 0x100, 0x80000301, 0x00000003, 0x0000004c,
                       0x00000040, 0x0);
    /* VDSP.dma */
    init_qos_by_module(iobase, 0x180, 0x80000301, 0x00000003, 0x00000180,
                       0x00000040, 0x0);
#endif

    iobase = p2v(APB_GPV_HPIA_BASE);
    /* CPU1.prim */
    init_qos_by_module(iobase, 0x2100, 0x80000403, 0x00000003, 0x00000000,
                       0x00000010, 0x0);
    /* CPU2.prim */
    init_qos_by_module(iobase, 0x2180, 0x80000403, 0x00000003, 0x00000000,
                       0x00000010, 0x0);
    /* CPU1.2nd */
    init_qos_by_module(iobase, 0x2200, 0x80000403, 0x00000003, 0x00000090,
                       0x00000010, 0x0);
    /* CPU2.2nd */
    init_qos_by_module(iobase, 0x2280, 0x80000403, 0x00000003, 0x00000090,
                       0x00000010, 0x0);
    /* TCU */
    init_qos_by_module(iobase, 0x2300, 0x80000503, 0x00000003, 0x0000001C,
                       0x00000008, 0x0);
    /* gpu1.a */
    init_qos_by_module(iobase, 0x2480, 0x80000301, 0x00000003, 0x00000120,
                       0x00000080, 0x0);
    /* gpu1.b */
    init_qos_by_module(iobase, 0x2500, 0x80000301, 0x00000003, 0x00000120,
                       0x00000080, 0x0);
    /* gpu2 */
    init_qos_by_module(iobase, 0x2580, 0x80000301, 0x00000003, 0x00000120,
                       0x00000080, 0x0);

    iobase = p2v(APB_GPV_SEC_M_BASE);
    /* CE2 */
    init_qos_by_module(iobase, 0x480, 0x80000301, 0x00000003, 0x000000f5,
                       0x00000020, 0x0);
    /* ENET2 */
    init_qos_by_module(iobase, 0x500, 0x80000402, 0x00000003, 0x0000004c,
                       0x00000010, 0x0);
    /* MSHC1 */
    init_qos_by_module(iobase, 0x580, 0x80000301, 0x00000003, 0x000000f5,
                       0x00000020, 0x0);
    /* MSHC2 */
    init_qos_by_module(iobase, 0x600, 0x80000301, 0x00000003, 0x0000007a,
                       0x00000020, 0x0);
    /* MSHC3 */
    init_qos_by_module(iobase, 0x680, 0x80000301, 0x00000003, 0x0000007a,
                       0x00000020, 0x0);
    /* MSHC4 */
    init_qos_by_module(iobase, 0x700, 0x80000301, 0x00000003, 0x0000003d,
                       0x00000020, 0x0);
    /* DMA2 */
    init_qos_by_module(iobase, 0x780, 0x80000301, 0x00000003, 0x0000003d,
                       0x00000008, 0x0);
    /* DMA3 */
    init_qos_by_module(iobase, 0x800, 0x80000301, 0x00000003, 0x0000003d,
                       0x00000008, 0x0);
    /* DMA4 */
    init_qos_by_module(iobase, 0x880, 0x80000301, 0x00000003, 0x0000003d,
                       0x00000008, 0x0);
    /* DMA5 */
    init_qos_by_module(iobase, 0x900, 0x80000301, 0x00000003, 0x0000003d,
                       0x00000008, 0x0);
    /* DMA6 */
    init_qos_by_module(iobase, 0x980, 0x80000301, 0x00000003, 0x0000003d,
                       0x00000008, 0x0);
    /* DMA7 */
    init_qos_by_module(iobase, 0xa00, 0x80000301, 0x00000003, 0x0000003d,
                       0x00000008, 0x0);
    /* DMA8 */
    init_qos_by_module(iobase, 0xa80, 0x80000301, 0x00000003, 0x0000003d,
                       0x00000008, 0x0);

    iobase = p2v(APB_GPV_SEC_BASE);
    /* cr5_sec */
    init_qos_by_module(iobase, 0x100, 0x80000503, 0x00000003, 0x00000099,
                       0x00000008, 0x0);

    if (0 != RES_GATING_EN_SEC_MP_PLAT) {
        /* cr5_mp */
        init_qos_by_module(iobase, 0x180, 0x80000503, 0x00000003, 0x00000099,
                           0x00000008, 0x0);
    }

    /* safety core resources */
#if 0
    iobase = p2v(APB_GPV_SAF_BASE);
    /* cr5_saf.prim */
    init_qos_by_module(iobase, 0x100, 0x80000503, 0x00000003, 0x00000000,
                       0x00000008, 0x0);
    /* cr5_saf.perp */
    init_qos_by_module(iobase, 0x180, 0x80000503, 0x00000003, 0x00000099,
                       0x00000002, 0x0);
    /* ce1 */
    init_qos_by_module(iobase, 0x200, 0x80000301, 0x00000003, 0x0000007a,
                       0x00000010, 0x0);
    /* dma1 */
    init_qos_by_module(iobase, 0x280, 0x80000301, 0x00000003, 0x0000003d,
                       0x00000008, 0x0);
    /* enet1 */
    init_qos_by_module(iobase, 0x300, 0x80000402, 0x00000003, 0x0000004c,
                       0x00000010, 0x0);
#endif
}
