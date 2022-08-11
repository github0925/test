/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#ifndef __SOC_DEF_H__
#define __SOC_DEF_H__

typedef enum {
    MODULE_INVALID = 0,
    CORE1 = 1,
    CORE2,
    CORE3,
    CORE4,
    CPU_SAF = 1,
    CPU_SEC = 2,
    CPU_AP1 = 3,
    CPU_AP2 = 4,
    CPU_MP = 5,
    DOMAIN1 = 0x0C,
    DOMAIN2,
    CHIP,
    SYSTEM,
    SD_MMC_CTRL1 = 0x10,
    SD_MMC_CTRL2,
    SD_MMC_CTRL3,
    SD_MMC_CTRL4,
    SD_MMC_PHY1,
    SD_MMC_PHY2,

    MAILBOX = 0x20,
    FUSECTRL = 0x28,
    FUSE_CTRL1 = 0x28,

    UART1 = 0x30,
    UART2,
    UART3,
    UART4,
    UART5,
    UART6,
    UART7,
    UART8,
    UART9,
    UART10,
    UART11,
    UART12,
    UART13,
    UART14,
    UART15,
    UART16,
    UART17,
    UART18,
    UART19,
    UART20,

    TIMER1 = 0x50u,
    TIMER2,
    TIMER3,
    TIMER4,
    TIMER5,
    TIMER6,
    TIMER7,
    TIMER8,

    WDT1 = 0x60u,
    WDT2,
    WDT3,
    WDT4,

    CANFD1 = 0x70u,
    CANFD2,
    CANFD3,
    CANFD4,
    CANFD5,
    CANFD6,
    CANFD7,
    CANFD8,
    CANFD9,
    CANFD10,
    CANFD11,
    CANFD12,
    CANFD13,
    CANFD14,
    CANFD15,
    CANFD16,

    OSPI1 = 0x80u,
    OSPI_CTRL1 = 0x80,
    OSPI2 = 0x81,
    OSPI_CTRL2 = 0x81,

    SPI1 = 0x90u,
    SPI2,
    SPI3,
    SPI4,
    SPI5,
    SPI6,
    SPI7,
    SPI8,

    I2C1 = 0xA0u,
    I2C2,
    I2C3,
    I2C4,
    I2C5,
    I2C6,
    I2C7,
    I2C8,
    I2C9,
    I2C10,
    I2C11,
    I2C12,
    I2C13,
    I2C14,
    I2C15,
    I2C16,

    DMA1 = 0xB0u,
    DMA2,
    DMA3,
    DMA4,
    DMA5,
    DMA6,
    DMA7,
    DMA8,

    PWM1 = 0xC0u,
    PWM2,
    PWM3,
    PWM4,
    PWM5,
    PWM6,
    PWM7,
    PWM8,

    CE1 = 0xD0u,
    CE2,
    CRYPTO_ENG1 = 0xD0,
    CRYPTO_ENG2,

    GPIO1 = 0x110,
    GPIO2,
    GPIO3,
    GPIO4,
    GPIO5,

    ENET1 = 0x120,
    ENET2,
    USB_CTRL1 = 0x128,
    USB_CTRL2,
    PCIE1X = 0x130,
    PCIE2X,

    GIC = 0x138,

    WDOG1 = 0x140,
    WDOG2,
    WDOG3,
    WDOG4,
    WDOG5,
    WDOG6,
    WDOG7,
    WDOG8,

    DDR_SS = 0x150,

    CPU1_SS = 0x158,
    CPU2_SS = 0x151,

    MODULE_END,

} module_e;

typedef enum {
    SDMMC_IDENT = 0,    /* 400 KHz*/
    SDMMC_25MHz,
    SDMMC_50MHz,
    SDMMC_25MHz_DDR,
    SDMMC_50MHz_DDR,

    SPI_FREQ0 = 10,     /* 20MHz */
    SPI_FREQ1,          /* 40Hz */
    SPI_FREQ2,          /* 80MHz */
    SPI_FREQ3,          /* 133MHz */
    SPI_FREQ4,          /* 160MHz */
    SPI_FREQ0_DIV = 20,  /* SCLK on 20MHz, but ref clk shall be much higher */
    SPI_FREQ1_DIV = 21,  /* 40MHz */
    SPI_FREQ2_DIV = 22,  /* 100MHz */
    SPI_FREQ3_DIV = 23,

    UART_FREQ1 = 0x80,
    UART_FREQ2 = 0x81,
    UART_FREQ3 = 0x82,

    FREQ_SET_BY_PLAT = 0xF0,
    FREQ_DEFAULT = 0xF0,
    FREQ_SAFE = 0xF0,
    FREQ_INVALID = 0xFF,

    FREQ_MAX = 0xFFFFFFFFu,
} clk_freq_e;

/* RST_CYCLE_PERSIST0_FLAGs*/
#define FM_ROM_FLAG_ROM_SELFTEST    (0x03U << 0)
#define FLAG_RST_PERST_XCPT         (0x01U << 4)
/*RST_CYCLE_PERSIST0_FLAGs*/
#define BM_ROM_FLAG_HANDOVERED      (0x01U << 31)

#endif  /* __SOC_DEF_H__ */
