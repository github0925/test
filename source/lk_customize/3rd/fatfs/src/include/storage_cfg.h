
#include "spi_nor_hal.h"
#include "mmc_hal.h"
#include "storage_device.h"
#include "chip_res.h"

struct storage_device_cfg{
    uint32_t device_type;
    uint32_t res_idex;
    union {
        struct mmc_cfg mmc_cfg;
        struct spi_nor_cfg ospi_cfg;
    } cfg;
    uint64_t offset;
};

static struct storage_device_cfg mmc1 =
{
    .device_type = MMC,
    .res_idex = RES_MSHC_SD1,
    .cfg = {
        .mmc_cfg = {
            .voltage = MMC_VOL_1_8,
            .max_clk_rate = MMC_CLK_200MHZ,
            .bus_width = MMC_BUS_WIDTH_8BIT,
            .hs400_support = 1,
        },
    },
};

static struct storage_device_cfg mmc2 =
{
    .device_type = MMC,
    .res_idex = RES_MSHC_SD2,
    .cfg = {
        .mmc_cfg = {
            .voltage = MMC_VOL_1_8,
            .max_clk_rate = MMC_CLK_100MHZ,
            .bus_width = MMC_BUS_WIDTH_8BIT,
        },
    },
};

static struct storage_device_cfg mmc3 =
{
    .device_type = MMC,
    .res_idex = RES_MSHC_SD3,
    .cfg = {
        .mmc_cfg = {
            .voltage = MMC_VOL_3_3,
            .max_clk_rate = MMC_CLK_25MHZ,
            .bus_width = MMC_BUS_WIDTH_4BIT,
        },
    },
};

static struct storage_device_cfg mmc4 =
{
    .device_type = MMC,
    .res_idex = RES_MSHC_SD4,
    .cfg = {
        .mmc_cfg = {
            .voltage = MMC_VOL_3_3,
            .max_clk_rate = MMC_CLK_25MHZ,
            .bus_width = MMC_BUS_WIDTH_4BIT,
        },
    },
};


static struct storage_device_cfg ospi1 =
{
    .device_type = OSPI,
    .res_idex = RES_OSPI_REG_OSPI1,
    .cfg = {
        .ospi_cfg = {
            .cs = SPI_NOR_CS0,
            .bus_clk = SPI_NOR_CLK_25MHZ,
            .octal_ddr_en = 1,
        },
    },
};

static struct storage_device_cfg ospi2 =
{
    .device_type = OSPI,
    .res_idex = RES_OSPI_REG_OSPI2,
    .cfg = {
        .ospi_cfg = {
            .cs = SPI_NOR_CS0,
            .bus_clk = SPI_NOR_CLK_25MHZ,
            .octal_ddr_en = 0,
        },
    },
};

static struct storage_device_cfg memdisk1 =
{
    .device_type = MEMDISK,
    .res_idex = RES_OSPI_OSPI1,
};

static struct storage_device_cfg memdisk2 =
{
    .device_type = MEMDISK,
    .res_idex = 0,
};
