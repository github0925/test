/*
* disp_panels.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 10/21/2019 BI create this file
*/
#include "disp_hal.h"
#include "disp_panels.h"
//#include <wrapper.h>
//#include <tx_lib.h>

/*set lcds for each display*/

/* avoid an error with IAR compiler */
struct sdm_panel dumb_lcd = {
    .panel_name = "dumb_lcd",
};

/*cluster*/
struct sdm_panel *display_0_panels[] = {
#if defined(DEFAULT)
    &lvds_youda_1920x720_lcd,
#endif

#if defined(DEFAULT_4K)
    &lvds_dv489fbm_3840x720_lcd,
#endif

#if defined(SERDES)
    &lvds_hsd123_serdes_1920x720_lcd,
#endif

#if defined(E_SERDES)
	&lvds_hsd123_serdes_1920x720_lcd,
#endif

#if defined(M_DEFAULT)
    &lvds_youda_1920x720_lcd,
#endif

#if defined(M_SERDES)
    &lvds_hsd123_serdes_1920x720_lcd,
#endif

#if defined(V9TS_DEFAULT)
    &mipi_hsd123_serdes_1920x720_v9ts_lcd,
#endif

#if defined(V_DEFAULT)
#if defined(MIPI_HSD123_SERDES_1920X720_V9_LCD)
    &mipi_hsd123_serdes_1920x720_v9_lcd,
#endif
#endif

#if defined(ICL02)
    &lvds_ICL02_hsd123_serdes_1920x720_lcd,
#endif

#if defined(X9U_B)
    &lvds_hsd123_serdes_1920x720_lcd,
#endif

#if defined(X9U_A)
    &lvds_hsd123_serdes_1920x720_lcd,
#endif

   &dumb_lcd,

};

/*ivi-master*/
struct sdm_panel *display_1_panels[] = {
#if defined(DEFAULT)
    &lvds_atk10_1_1280x800_lcd,
#endif

#if defined(SERDES)
    &mipi_hsd123_serdes_1920x720_lcd,
#endif

#if defined(M_DEFAULT)
    &lvds_youda_1920x720_lcd,
#endif

#if defined(E_SERDES)
    &lvds_hsd123_serdes_1920x720_lcd,
#endif

#if defined(M_SERDES)
    &mipi_hsd123_serdes_1920x720_lcd,
#endif

#if defined(V_DEFAULT)
#if defined(MIPI_HSD123_SERDES_1920X720_V9_LCD)
    &mipi_hsd123_serdes_1920x720_v9_lcd,
#endif

#if defined(MIPI_KD070_SERDES_1024X600_LCD)
    &mipi_kd070_serdes_1024x600_lcd,
#endif
#if defined(MIPI_LT9611_TO_HDMI_1920X1080_LCD)
    &mipi_lt9611_to_hdmi_1920x1080_lcd,
#endif
#if defined(LVDS_MTF101_SERDES_1920X1200_LCD)
    &lvds_mtf101_serdes_1920x1200_lcd,
#endif
#endif //end #if defined(V_DEFAULT)

#if defined(V9TS_DEFAULT)
	&mipi_hsd123_serdes_1920x720_v9ts_lcd,
#endif

#if defined(DEMO_2_BACK)
    &mipi_hsd101_serdes_1280x800_lcd,
#endif

#if defined(ICL02)
    &mipi_ICL02_hsd162_serdes_2608x720_lcd,
#endif

#if defined(X9U_B)
    &mipi_hsd123_serdes_1920x720_lcd,
#endif

#if defined(X9U_A)
    &mipi_hsd123_serdes_1920x720_lcd,
#endif

#if defined(BF200)
    &mipi_lt9611_to_hdmi_1920x1080_lcd,
#endif

   &dumb_lcd,

};

/*ivi-slave*/
struct sdm_panel *display_2_panels[] = {
#if defined(DEFAULT)
    &lvds_atk10_1_1280x800_lcd,
#endif

#if defined(SERDES)
    &mipi_hsd123_serdes_1920x720_2_lcd,
#endif

#if defined(DEMO_2_BACK)
    &mipi_hsd101_serdes_1280x800_2_lcd,
#endif

#if defined(ICL02)
    &mipi_ICL02_hsd123_serdes_1920x720_lcd,
#endif

#if defined(X9U_B)
    &mipi_hsd123_serdes_1920x720_2_lcd,
#endif

#if defined(X9U_A)
    &mipi_hsd123_serdes_1920x720_2_lcd,
#endif

#if defined(BF200)
    &parallel_gm7123_to_vga_1920x1080_lcd,
#endif

   &dumb_lcd,

};

/*control panel*/
struct sdm_panel *display_3_panels[] = {
#if defined(SERDES)
    &lvds_hsd123_serdes_1920x720_lcd,
#endif

#if defined(M_SERDES)
    &lvds_hsd123_serdes_1920x720_lcd,
#endif

#if defined(S3SIN1)
    &mipi_hsd123_serdes_1920x720_lcd,
#endif

#if defined(S3SIN1_1920X1080)
    &lvds_hsd156_serdes_1920x1080_lcd,
#endif

#if defined(DEMO_2_BACK)
    &lvds_hsd101_serdes_1280x800_lcd,
#endif

#if defined(X9U_B)
    &lvds_hsd123_serdes_1920x720_lcd,
#endif

#if defined(X9U_A)
    &lvds_hsd123_serdes_1920x720_lcd,
#endif

   &dumb_lcd,

};

/*hud*/
struct sdm_panel *display_4_panels[] = {
#if defined(SERDES)
    &lvds_hsd123_serdes_1920x720_lcd,
#endif

#if defined(M_SERDES)
    &lvds_hsd123_serdes_1920x720_lcd,
#endif

#if defined(S3SIN1)
    &mipi_hsd123_serdes_1920x720_lcd,
#endif

#if defined(DEMO_2_BACK)
    &lvds_hsd101_serdes_1280x800_lcd,
#endif

#if defined(X9U_B)
    &lvds_hsd101_serdes_1280x800_lcd,
#endif

#if defined(X9U_A)
    &lvds_kd070_serdes_1024x600_lcd,
#endif

   &dumb_lcd,

};


struct sdm_panel **registered_panels[] = {
    display_0_panels,
    display_1_panels,
    display_2_panels,
    display_3_panels,
    display_4_panels,
};

const unsigned char adv_init_table[] = {
  0x41, 0x00,
  0x98, 0x03,
  0x9A, 0xE0,
  0x9C, 0x30,
  0x9D, 0x61,
  0xA2, 0xA4,
  0xA3, 0xA4,
  0xE0, 0xD0,
  0x55, 0x12,
  0xF9, 0x00,
  0x9D, 0x61,
  0x15, 0x00,
  0x48, 0x00,
  0x16, 0x36,
  0x17, 0x00,
  0x18, 0x00,
  0xAF, 0x06,
  0x4C, 0x00,
  0x40, 0x00,
  0x00, 0x00
};

int adv_init(unsigned char bus_id)
{
#if 0
    int i = 0;
    unsigned char val;
    unsigned short Rev0;

    HAL_I2CBusReadByte(bus_id, TXDEV_MAIN_MAP,
            TXHAL_REG_OFFSET(0, 0x00),
            (unsigned char *)&Rev0);
    LOGD("adv chip revision = %x\n", Rev0);

    while (adv_init_table[i]) {
        HAL_I2CBusWriteByte(bus_id, TXDEV_MAIN_MAP,
                TXHAL_REG_OFFSET(0, adv_init_table[i]),
                adv_init_table[i+1]);
        HAL_I2CBusReadByte(bus_id, TXDEV_MAIN_MAP,
                TXHAL_REG_OFFSET(0, adv_init_table[i]),
                &val);
        i += 2;
    }
#endif
    return 0;
}

int send_i2c_command(struct sdm_panel *panel) {
    LOGD("request a i2c handle");
    LOGD("send i2c command %p\n", panel->cmd_data);
    LOGD("release i2c handle");
    return -1;
}

int send_spi_command(struct sdm_panel *panel) {
    LOGD("send spi command %p\n", panel->cmd_data);
    return -1;
}

int sdm_connect_panel(display_handle *handle, int sub_id, struct sdm_panel *panel) {
    int ret = hal_sdm_panel_connect(handle, sub_id, panel);

    switch (panel->if_type)
    {
    case IF_TYPE_I2C:
        break;
    case IF_TYPE_SPI:
        break;
    case IF_TYPE_HDMI:
        ret = adv_init(sub_id);
        break;
    case IF_TYPE_LVDS:
    case IF_TYPE_DSI:
        break;
    case IF_TYPE_PARALLEL:
        break;
    default:
        LOGD("invalid panel if_type: %d\n", panel->if_type);
        return -2;
    }

    if (ret) {
        LOGD("panel init failed in driver: %d\n", ret);
        return -1;
    }

    switch (panel->cmd_intf)
    {
    case IF_TYPE_I2C:
       	return send_i2c_command(panel);
    case IF_TYPE_SPI:
        return send_spi_command(panel);
    default:
        break;
    }

    //LOGD("panel %s conncted success\n", panel->panel_name);

    return 0;
}

int sdm_panel_probe(display_handle *handle, int sub_id, struct sdm_panel *panels[], int n_panels) {
    int found = -1;
	//LOGD("n_panels count %d", n_panels);

    for (int i = 0; i < n_panels; i++)
    {
        struct sdm_panel *p = panels[i];
        //LOGD("check panel %s...\n", p->panel_name);
        if (0 == sdm_connect_panel(handle, sub_id, p)) {
            found = i;
            break;
        }
    }

    if (found == -1) {
        LOGD("can not found a valid panel");
        return -1;
    }

    return found;
}

int sdm_panel_disconnect(display_handle *handle, int sub_id, struct sdm_panel *panel) {
    return hal_sdm_panel_disconnect(handle, sub_id, panel);
}
