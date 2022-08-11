#include "app_lin_cfg.h"
#include "Lin_Cfg.h"
#include "chip_res.h"
#include "Lin_GeneralTypes.h"


const char *chn[5] = {"chn0","chn1","chn2","chn3","chn4"}; 

static uint8 lindata[][LIN_SEND_DATA_LEN] = {

	{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
	
	{0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10}
	
};


Lin_PduType linPduInfo[]={

	{
		.Pid= 0x05,
		.Cs = LIN_CLASSIC_CS, 
		.Drc= LIN_MASTER_RESPONSE,
		.Dl = LIN_SEND_DATA_LEN, 
		.SduPtr = lindata[0],

	},
		
	{
		 .Pid= 0x09,
		 .Cs = LIN_CLASSIC_CS, 
		 .Drc= LIN_SLAVE_RESPONSE,
		 .Dl = LIN_SEND_DATA_LEN, 
		 .SduPtr = lindata[1],

	},

};


Bool get_Lin_PduType(uint8 typedNums,Lin_PduType **pduType){

 	Bool ret = true;
	
	if(typedNums >= sizeof(lindata) / sizeof(lindata[0])){

		ret = false;

	}

	printf("typedNums = %d \n",typedNums);
	
	*pduType = &linPduInfo[typedNums];

	return ret;

}


