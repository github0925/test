/*
* app_wdg.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: wdg samplecode.
*
* Revision History:
* -----------------
*/
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <arch.h>
#include "Lin.h"
#include "app_lin_cfg.h"
#include "Lin_GeneralTypes.h"

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

#include "hal_port.h"
#include "res.h"
#include "chip_res.h"

#include <lib/console.h>
static int cmd_Lin_SendFrame(int argc, const cmd_args *argv);
static int cmd_Lin_Wakeup(int argc, const cmd_args *argv);
static int cmd_Lin_WakeupInternal(int argc, const cmd_args *argv);
static int cmd_Lin_GoToSleep(int argc, const cmd_args *argv);
static int cmd_Lin_GoToSleepInternal(int argc, const cmd_args *argv);
static int cmd_Lin_GetStatus(int argc, const cmd_args *argv);


STATIC_COMMAND_START

STATIC_COMMAND("lin_sendframe",
               "chn0:sendframe to channel 0, chn1:sendframe to channel 1, -all:chn0&chn1",
               (console_cmd)&cmd_Lin_SendFrame)
STATIC_COMMAND("lin_wakeup",
               "chn0:wakeup to channel 0, chn1:wakeup to channel 1, -all:chn0&chn1",
               (console_cmd)&cmd_Lin_Wakeup)
STATIC_COMMAND("lin_wakeupinternal",
               "chn0:wakeupinternal to channel 0, chn1:wakeupinternal to channel 1, -all:chn0&chn1",
               (console_cmd)&cmd_Lin_WakeupInternal)
STATIC_COMMAND("lin_gosleep",
               "chn0:gosleep to channel 0, chn1:gosleep to channel 1, -all:chn0&chn1",
               (console_cmd)&cmd_Lin_GoToSleep)
STATIC_COMMAND("lin_gosleepInternal",
               "chn0:gosleepInternal to channel 0, chn1:gosleepInternal to channel 1, -all:chn0&chn1",
               (console_cmd)&cmd_Lin_GoToSleepInternal)
STATIC_COMMAND("lin_Getstatus",
               "chn0:Getstatus to channel 0, chn1:Getstatus to channel 1, -all:chn0&chn1",
               (console_cmd)&cmd_Lin_GetStatus)
STATIC_COMMAND_END(lin_demo);

const Port_PinType PIN_GPIO_A10_M2_UART4_TX = PortConf_PIN_GPIO_A10;
const Port_PinModeType MODE_GPIO_A10_M2_UART4_TX = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT2),
};

const Port_PinType PIN_GPIO_A11_M2_UART4_RX = PortConf_PIN_GPIO_A11;
const Port_PinModeType MODE_GPIO_A11_M2_UART4_RX = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_UP ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_ALT2),
};

const Port_PinType PIN_GPIO_B4_M1_UART5_TX = PortConf_PIN_GPIO_B4;
const Port_PinModeType MODE_GPIO_B4_M1_UART5_TX = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT1),
};

const Port_PinType PIN_GPIO_B5_M1_UART5_RX = PortConf_PIN_GPIO_B5;
const Port_PinModeType MODE_GPIO_B5_M1_UART5_RX  = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_UP ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_ALT1),
};

const Port_PinType PIN_GPIO_B6_M1_UART6_TX = PortConf_PIN_GPIO_B6;
const Port_PinModeType MODE_GPIO_B6_M1_UART6_TX = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT1),
};

const Port_PinType PIN_GPIO_B7_M1_UART6_RX = PortConf_PIN_GPIO_B7;
const Port_PinModeType MODE_GPIO_B7_M1_UART6_RX = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_UP ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_ALT1),
};

const Port_PinType PIN_GPIO_B8_M2_UART7_TX = PortConf_PIN_GPIO_B8;
const Port_PinModeType MODE_GPIO_B8_M2_UART7_TX = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT2),
};

const Port_PinType PIN_GPIO_B9_M2_UART7_RX = PortConf_PIN_GPIO_B9;
const Port_PinModeType MODE_GPIO_B9_M2_UART7_RX = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_UP ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_ALT2),
};

const Port_PinType PIN_GPIO_B10_M2_UART8_TX = PortConf_PIN_GPIO_B10;
const Port_PinModeType MODE_GPIO_B10_M2_UART8_TX = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT2),
};

const Port_PinType PIN_GPIO_B11_M2_UART8_RX = PortConf_PIN_GPIO_B11;
const Port_PinModeType MODE_GPIO_B11_M2_UART8_RX = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_UP ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_ALT2),
};

extern const domain_res_t g_iomuxc_res;

void uart_lin_port_setup(void)
{
    static void *port_handle;
    bool ioret;

    // Port setup
    ioret = hal_port_creat_handle(&port_handle, g_iomuxc_res.res_id[0]);
    printf("%s: create handle: %d\n", __func__, ioret);


    ioret = hal_port_set_pin_mode(port_handle, PIN_GPIO_A10_M2_UART4_TX,
                                  MODE_GPIO_A10_M2_UART4_TX);

    ioret = hal_port_set_pin_mode(port_handle, PIN_GPIO_A11_M2_UART4_RX,
                                  MODE_GPIO_A11_M2_UART4_RX);

    ioret = hal_port_set_pin_mode(port_handle, PIN_GPIO_B4_M1_UART5_TX,
                                  MODE_GPIO_B4_M1_UART5_TX);
    ioret = hal_port_set_pin_mode(port_handle, PIN_GPIO_B5_M1_UART5_RX,
                                  MODE_GPIO_B5_M1_UART5_RX);

    ioret = hal_port_set_pin_mode(port_handle, PIN_GPIO_B6_M1_UART6_TX,
                                  MODE_GPIO_B6_M1_UART6_TX);
    ioret = hal_port_set_pin_mode(port_handle, PIN_GPIO_B7_M1_UART6_RX,
                                  MODE_GPIO_B7_M1_UART6_RX);

    ioret = hal_port_set_pin_mode(port_handle, PIN_GPIO_B8_M2_UART7_TX,
                                  MODE_GPIO_B8_M2_UART7_TX);
    ioret = hal_port_set_pin_mode(port_handle, PIN_GPIO_B9_M2_UART7_RX,
                                  MODE_GPIO_B9_M2_UART7_RX);

    ioret = hal_port_set_pin_mode(port_handle, PIN_GPIO_B10_M2_UART8_TX,
                                  MODE_GPIO_B10_M2_UART8_TX);
    ioret = hal_port_set_pin_mode(port_handle, PIN_GPIO_B11_M2_UART8_RX,
                                  MODE_GPIO_B11_M2_UART8_RX);

    hal_port_release_handle(&port_handle);

}

static void app_lin_init(void)
{

	uart_lin_port_setup();  //uart mode

    Lin_Init((const Lin_ConfigType *)&lin_config[0]);

}

static int cmd_Lin_SendFrame(int argc, const cmd_args *argv)
{
	Lin_PduType *PduType=NULL_PTR;

    if (argc == 2) {

		for(uint8 ch=0;ch<LIN_IFC_CHN_MAX;ch++){

			if(strcmp(argv[1].str,chn[ch]) == 0){

				if(true != get_Lin_PduType(ch,&PduType)){

					printf("channel num is over\n");

					return 0;
				}
							
				Lin_SendFrame(ch,PduType);

				printf("lin channal %d send frame\n",ch);

				return 0;

			}

		}		
        
    }

	printf("no lin channel match\n");
	
    return 0;
}

static int cmd_Lin_Wakeup(int argc, const cmd_args *argv)
{
	 if (argc == 2) {

		for(uint8 ch=0;ch<LIN_IFC_CHN_MAX;ch++){

			if(strcmp(argv[1].str,chn[ch]) == 0){

				Lin_Wakeup(ch);

				printf("lin channal %d wakeup\n",ch);

				return 0;

			}

		}		
        
    }

	printf("no lin channel match\n");
	
    return 0;
}

static int cmd_Lin_WakeupInternal(int argc, const cmd_args *argv)
{
	if (argc == 2) {
	
	   for(uint8 ch=0;ch<LIN_IFC_CHN_MAX;ch++){

		   if(strcmp(argv[1].str,chn[ch]) == 0){

			   Lin_WakeupInternal(ch);

			   printf("lin channal %d wakeupInternal\n",ch);

			   return 0;

		   	}

	   	}	   
		   
	}
	
   printf("no lin channel match\n");
   
   return 0;
}

static int cmd_Lin_GoToSleep(int argc, const cmd_args *argv)
{
	if (argc == 2) {
		
	   for(uint8 ch=0;ch<LIN_IFC_CHN_MAX;ch++){

		   if(strcmp(argv[1].str,chn[ch]) == 0){

			   Lin_GoToSleep(ch);

			   printf("lin channal %d go to sleep\n",ch);

			   return 0;

		   }

	   }	   
	   
   }
		
   printf("no lin channel match\n");
   
   return 0;

}

static int cmd_Lin_GoToSleepInternal(int argc, const cmd_args *argv)
{
	if (argc == 2) {
		
	   for(uint8 ch=0;ch<LIN_IFC_CHN_MAX;ch++){

		   if(strcmp(argv[1].str,chn[ch]) == 0){

			   Lin_GoToSleepInternal(ch);

			   printf("lin channal %d go to sleep interanl\n",ch);

			   return 0;

		   }

	   }	   
	   
   }
			
   printf("no lin channel match\n");
   
   return 0;
	   
}

static void Lin_Print(uint8 *ptr)
{

	uint8 i;

    printf("  data: ");

    for (i = 0; i < 9; i++) {
        printf("%x ", *ptr++);
    }

    printf("\r");
}

static int cmd_Lin_GetStatus(int argc, const cmd_args *argv)
{
	uint8 *Lin_SduPtr;

	if (argc == 2) {
				
	   for(uint8 ch=0;ch<LIN_IFC_CHN_MAX;ch++){

		   if(strcmp(argv[1].str,chn[ch]) == 0){
		   	
		   	   printf("lin channal %d getStatus\n",ch);

			   if(E_NOT_OK != Lin_GetStatus(ch, &Lin_SduPtr)){

			   		Lin_Print(Lin_SduPtr);

				}else{

					printf("getStatus error\n");				

				}
				Lin_Print(Lin_SduPtr);
				 return 0;

		   }

	   }	   
   }

   printf("no lin channel match\n");

   return 0;
}

static void cmd_Lin_Init(const struct app_descriptor *app){

	app_lin_init();

}


APP_START(lin_app)
.init = cmd_Lin_Init,
.flags = 0,

APP_END

