/*
 * Copyright (c) 2019 Semidrive Inc.
 *
 * SDPE Manager Sample Application.
 */
#include <assert.h>
#include <app.h>
#include <lib/console.h>
#include <kernel/event.h>

#include "__regs_base.h"
#include "spi_nor_hal.h"
#include "partition_parser.h"
#include "storage_device.h"

#include "Can.h"
#undef ERPC_TYPE_DEFINITIONS
#include "Lin.h"
#include "can_cfg.h"
#include "hal_port.h"
#include "hal_dio.h"
#include "tca9539.h"
#include "chip_res.h"
#include "sdrpc.h"
#include "dcf.h"

#ifdef SUPPORT_SDPE_RPC
#include "sdpe_ctrl_service.h"
#include "sdpe_rpc.h"
#else
#include "sdpe_ctrl.h"
#endif

#include "image_cfg.h"

#if SUPPORT_BOARDINFO
#include "boardinfo_hwid_usr.h"
#endif

extern void start_rpmsg_service(void);
#ifdef SUPPORT_3RD_ERPC
extern bool vircom_init(void);
#endif

extern struct Can_ControllerConfig gCan_CtrllerConfig[];
extern struct Can_MBConfig gCan_RxMBCfg[];
extern struct Can_MBConfig gCan_TxMBCfg[];
extern const Lin_ConfigType lin_config;

extern const domain_res_t g_iomuxc_res;
extern const domain_res_t g_gpio_res;

static struct spi_nor_cfg g_ospi_cfg = {
    .cs = SPI_NOR_CS0,
    .bus_clk = SPI_NOR_CLK_25MHZ,
    .octal_ddr_en = 0,
};

static const Port_PinModeType PIN_GPIO_A4_I2C3_CLK = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT2),
};

static const Port_PinModeType PIN_GPIO_A5_I2C3_SDA  = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT2),
};

#if TARGET_REFERENCE_G9
static void i2c_port_setup(void)
{
    void *port_handle;

    hal_port_creat_handle(&port_handle, g_iomuxc_res.res_id[0]);

    if (port_handle) {
        /* I2C3 */
        hal_port_set_pin_mode(port_handle, PortConf_PIN_GPIO_A4, PIN_GPIO_A4_I2C3_CLK);
        hal_port_set_pin_mode(port_handle, PortConf_PIN_GPIO_A5, PIN_GPIO_A5_I2C3_SDA);
        hal_port_release_handle(&port_handle);
    }
    else {
        assert(false);
    }
}

static void sdpe_port_init(void)
{
    struct tca9539_device *pd;
    int board_ver = 2;  /* default A02 */

#if SUPPORT_BOARDINFO
    board_ver = get_part_id(PART_BOARD_ID_MIN);
    char hwid[32];
    dprintf(INFO, "Board version: %s\n",
            get_hwid_friendly_name(hwid, 32));
#endif

    i2c_port_setup();

    pd = tca9539_init(3, 0x74);
    assert(pd);

#ifdef SCI4_USED
    /* GPIO A10/A11 connected to LIN4 transceiver. */
    pd->ops.output_enable(pd, 5);   /* P04 SW_UART_LIN */
    pd->ops.output_val(pd, 5, 1);
#endif

#if defined SCI1_USED || defined SCI2_USED
    if (board_ver == 3) {
        pd->ops.output_enable(pd, 1);   /* LIN0_EN */
        pd->ops.output_val(pd, 1, 1);
        pd->ops.output_enable(pd, 6);   /* LIN1_EN */
        pd->ops.output_val(pd, 6, 1);
    }
    else {
        /* GPIO_B8/B9/B10/B11 connected to LIN 1/2 transceiver. */
        pd->ops.output_enable(pd, 6);
        pd->ops.output_val(pd, 6, 1);
    }
#endif

    /* CAN1 */
    pd->ops.output_enable(pd, 10);
    pd->ops.output_enable(pd, 11);
    pd->ops.output_enable(pd, 12);
    pd->ops.output_val(pd, 10, 1);  /* CAN1_EN */
    pd->ops.output_val(pd, 11, 1);  /* CAN1_NTSB */
    pd->ops.output_val(pd, 12, 1);  /* CAN1_NERR */

    tca9539_deinit(pd);
}
#else
static const Port_PinModeType PIN_GPIO_D0_I2C9_CLK = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT1),
};

static const Port_PinModeType PIN_GPIO_D1_I2C9_SDA  = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT1),
};

static void i2c_port_setup(void)
{
    void *port_handle;

    hal_port_creat_handle(&port_handle, g_iomuxc_res.res_id[0]);

    if (port_handle) {
        /* I2C3 */
        hal_port_set_pin_mode(port_handle, PortConf_PIN_GPIO_A4, PIN_GPIO_A4_I2C3_CLK);
        hal_port_set_pin_mode(port_handle, PortConf_PIN_GPIO_A5, PIN_GPIO_A5_I2C3_SDA);
        /* I2C9 */
        hal_port_set_pin_mode(port_handle, PortConf_PIN_GPIO_D0, PIN_GPIO_D0_I2C9_CLK);
        hal_port_set_pin_mode(port_handle, PortConf_PIN_GPIO_D1, PIN_GPIO_D1_I2C9_SDA);
        hal_port_release_handle(&port_handle);
    }
    else {
        assert(false);
    }
}

static void sdpe_port_init(void)
{
    struct tca9539_device *pd;

    i2c_port_setup();

    pd = tca9539_init(3, 0x74); /* I2C3 */
    ASSERT(pd);

    /* CANFD13 */
    pd->ops.output_enable(pd, 3); /* P02 CANFD13_EN_SAFETY */
    pd->ops.output_val(pd, 3, 1);
    pd->ops.output_enable(pd, 4); /* P03 CANFD13_STDBY_SAFETY */
    pd->ops.output_val(pd, 4, 1);

    /* CANFD14 */
    pd->ops.output_enable(pd, 7); /* P06 CANFD14_STDBY_SAFETY */
    pd->ops.output_val(pd, 7, 1);
    pd->ops.output_enable(pd, 8); /* P07 CANFD14_EN_SAFETY */
    pd->ops.output_val(pd, 8, 1);

    /* CANFD15 */
    pd->ops.output_enable(pd, 11); /* P12 CANFD15_EN_SAFETY */
    pd->ops.output_val(pd, 11, 1);
    pd->ops.output_enable(pd, 12); /* P13 CANFD15_STDBY_SAFETY */
    pd->ops.output_val(pd, 12, 1);

    /* CANFD20 */
    pd->ops.output_enable(pd, 15); /* P16 CANFD20_EN_SAFETY */
    pd->ops.output_val(pd, 15, 1);
    pd->ops.output_enable(pd, 16); /* P17 CANFD20_STDBY_SAFETY */
    pd->ops.output_val(pd, 16, 1);

    tca9539_deinit(pd);

    struct can_transceiver_config {
        Port_PinType    standby;
        Port_PinType    enable;
    } can_transceiver_configs[] = {
        {
            /* CANFD1 */
            .standby = PortConf_PIN_OSPI2_DATA1,
            .enable = PortConf_PIN_I2S_SC3_SCK,
        },
        {
            /* CANFD2 */
            .standby = PortConf_PIN_OSPI2_SS0,
            .enable = PortConf_PIN_OSPI2_DQS,
        },
        {
            /* CANFD3 */
            .standby = PortConf_PIN_OSPI2_DATA3,
            .enable = PortConf_PIN_I2S_SC3_WS,
        },
        {
            /* CANFD4 */
            .standby = PortConf_PIN_OSPI2_DATA7,
            .enable = PortConf_PIN_OSPI2_DATA0,
        },
        {
            /* CANFD9 */
            .standby = PortConf_PIN_RGMII2_RXD2,
            .enable = PortConf_PIN_RGMII2_RXD3,
        },
        {
            /* CANFD10 */
            .standby = PortConf_PIN_EMMC2_DATA3,
            .enable = PortConf_PIN_EMMC2_DATA0,
        },
        {
            /* CANFD11 */
            .standby = PortConf_PIN_EMMC2_DATA7,
            .enable = PortConf_PIN_EMMC2_DATA4,
        },
        {
            /* CANFD12 */
            .standby = PortConf_PIN_EMMC2_DATA5,
            .enable = PortConf_PIN_EMMC2_RESET_N,
        },
        {
            /* CANFD16 */
            .standby = PortConf_PIN_EMMC2_DATA6,
            .enable = PortConf_PIN_OSPI2_DATA4,
        },
#if !SUPPORT_SAMPLE_A
        {
            /* CANFD17 */
            .standby = PortConf_PIN_EMMC2_DATA2,
            .enable = PortConf_PIN_EMMC2_DATA1,
        },
#endif
        {
            /* CANFD18 */
            .standby = PortConf_PIN_RGMII2_TX_CTL,
            .enable = PortConf_PIN_RGMII2_RXC,
        },
        {
            /* CANFD19 */
            .standby = PortConf_PIN_EMMC2_CLK,
            .enable = PortConf_PIN_EMMC2_CMD,
        }
    };

    void *port_handle;
    void *dio_handle;

    hal_dio_creat_handle(&dio_handle, g_gpio_res.res_id[0]);
    hal_port_creat_handle(&port_handle, g_iomuxc_res.res_id[0]);

    assert(dio_handle);
    assert(port_handle);

    for (size_t i = 0;
            i < sizeof(can_transceiver_configs) / sizeof(can_transceiver_configs[0]);
            i++) {
        struct can_transceiver_config *config = &can_transceiver_configs[i];

        hal_port_set_pin_direction(port_handle, config->standby, PORT_PIN_OUT);
        hal_port_set_pin_direction(port_handle, config->enable, PORT_PIN_OUT);
        hal_dio_write_channel(dio_handle, config->standby, 1);
        hal_dio_write_channel(dio_handle, config->enable, 1);
    }

    hal_dio_release_handle(&dio_handle);
    hal_port_release_handle(&port_handle);
}
#endif

static bool load_route_table(vaddr_t dst, size_t size)
{
    struct storage_device *storage;
    struct partition_device *ptdev;
    uint64_t ptn;
    bool ret = false;

    storage = setup_storage_dev(OSPI, RES_OSPI_REG_OSPI1, &g_ospi_cfg);

    if (storage == NULL) {
        dprintf(ALWAYS, "setup_storage_dev error!\n");
        return false;
    }

    ptdev = ptdev_setup(storage, storage->get_erase_group_size(storage) * 2);

    if (ptdev == NULL) {
        dprintf(ALWAYS, "ptdev_setup error!\n");
        goto out_with_storage;
    }

    if (ptdev_read_table(ptdev)) {
        dprintf(ALWAYS, "ptdev_read_table error!\n");
        goto out_with_ptdev;
    }

    ptn = ptdev_get_offset(ptdev, "routing-table");

    if (!ptn) {
        dprintf(ALWAYS, "ptdev_get_offset error!\n");
        goto out_with_ptdev;
    }

    /* Cache is flushed by flash driver */
    storage->read(storage, ptn, (uint8_t *)dst, size);

    ret = true;

out_with_ptdev:
    ptdev_destroy(ptdev);

out_with_storage:
    storage_dev_destroy(storage);

    return ret;
}

static void sdpe_wait_res(void)
{
#if CONFIG_USE_SYS_PROPERTY
    system_property_wait_condition(DMP_ID_PLL_CLK_STATUS, 1);
#else

    while (!sdrpc_read_msg(NULL, COM_PORT_STATUS, NULL));

#endif
}

static void sdpe_can_init(void)
{
    dprintf(INFO, "Can_Init\n");

    for (size_t i = 0; i < gCan_Config.controllerCount; i++) {
        Can_ConfigType Can_Config = {
            .controllerCount = 1,
            .ctrllerCfg = &gCan_CtrllerConfig[i],
        };

        for (size_t j = 0; j < gCan_Config.rxCount; j++) {
            if (gCan_RxMBCfg[j].controllerId ==
                    gCan_CtrllerConfig[i].controllerId) {
                Can_Config.rxCount++;

                if (!Can_Config.rxMBCfg) {
                    Can_Config.rxMBCfg = &gCan_RxMBCfg[j];
                }
            }
        }

        for (size_t j = 0; j < gCan_Config.txCount; j++) {
            if (gCan_TxMBCfg[j].controllerId ==
                    gCan_CtrllerConfig[i].controllerId) {
                Can_Config.txCount++;

                if (!Can_Config.txMBCfg) {
                    Can_Config.txMBCfg = &gCan_TxMBCfg[j];
                }
            }
        }

        dprintf(INFO, "Can_Init %d\n", i);
        Can_Init(&Can_Config);
    }

    /* Send configuration paramenter ending message. */
    Can_ConfigType ending = {
        .controllerCount = 0
    };
    Can_Init(&ending);

    for (uint8_t i = 0; i < gCan_Config.controllerCount; i++) {
        dprintf(INFO, "Can_SetControllerMode %d\n", i);
        Can_SetControllerMode(gCan_Config.ctrllerCfg[i].controllerId,
                              CAN_CS_STARTED);
    }
}

static void sdpe_lin_init(void)
{
    dprintf(INFO, "Lin_Init\n");
    Lin_Init(&lin_config);

#ifdef SCI1_USED
    dprintf(INFO, "Lin_WakeupInternal 1\n");
    Lin_WakeupInternal(LIN_IFC_SCI1);
#endif

#ifdef SCI2_USED
    dprintf(INFO, "Lin_WakeupInternal 2\n");
    Lin_WakeupInternal(LIN_IFC_SCI2);
#endif

#ifdef SCI3_USED
    dprintf(INFO, "Lin_WakeupInternal 3\n");
    Lin_WakeupInternal(LIN_IFC_SCI3);
#endif

#ifdef SCI4_USED
    dprintf(INFO, "Lin_WakeupInternal 4\n");
    Lin_WakeupInternal(LIN_IFC_SCI4);
#endif
}

static void _sdpe_mgr_main(void)
{
    /* ssystem should have configured MUX and rstgen. */
    dprintf(INFO, "sdpe_mgr_main start\n");
    sdpe_wait_res();

    /* Load partition table */
    dprintf(INFO, "Load partition table\n");
    const vaddr_t route_table_buf = ROUTE_TAB_MEMBASE;
    const size_t route_table_size = ROUTE_TAB_MEMSIZE;

#if !SUPPORT_FAST_BOOT

    if (!load_route_table(route_table_buf, route_table_size)) {
        dprintf(CRITICAL, "load_route_table failed\n");
        return;
    }

#endif
    dprintf(INFO, "sdpe_port_init\n");
    sdpe_port_init();

#ifdef SUPPORT_SDPE_RPC
    sdpe_rpc_service_init();
#else
    dprintf(INFO, "start_rpmsg_service\n");
    start_rpmsg_service();

    dprintf(INFO, "vircom_init\n");

    if (false == vircom_init()) {
        dprintf(ALWAYS, "Initialize erpc failed\n");
        return;
    }

#endif

    sdpe_can_init();
    sdpe_lin_init();

    /* Notify SDPE to start. */
    dprintf(INFO, "sdpe_starting_routing\n");
    sdpe_start_routing((uint32_t)route_table_buf, route_table_size);
}

/**
 * @brief Start SDPE.
 */
int sdpe_mgr_main(int argc, const cmd_args *argv)
{
    _sdpe_mgr_main();
    return 0;
}

static void sdpe_mgr_entry(const struct app_descriptor *app, void *args)
{
    _sdpe_mgr_main();
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("sdpe_mgr", "SDPE Test", (console_cmd)&sdpe_mgr_main)
STATIC_COMMAND_END(sdpe_mgr);
#endif

APP_START(sdpe_mgr)
.flags = 0,
.entry = sdpe_mgr_entry,
APP_END
