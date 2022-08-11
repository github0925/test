/********************************************************
 *	    Copyright(c) 2019	Semidrive  Semiconductor    *
 *	    All rights reserved.                            *
 ********************************************************/

#define USB_STR1_LEN    20
#define USB_STR2_LEN    10
#define USB_STR3_LEN    34
#define USB_STR4_LEN    18

static unsigned short usb_str_manufacturer[] = {
    0x300 | USB_STR1_LEN,
    'S', 'E', 'M', 'I', 'D', 'R', 'I', 'V', 'E',
};

static unsigned short usb_str_prod[] = {
    0x300 | USB_STR2_LEN,
    '0', '0', '0', '0',
};

static unsigned short usb_str_serial[] = {
    0x300 | USB_STR3_LEN,
    '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0',
};

static unsigned char usb_str_os[] = {
    USB_STR4_LEN,
    0x30,   /* String Descriptor */
    'M', 0, 'S', 0, 'F', 0, 'T', 0, '1', 0, '0', 0, '0', 0,
    1,      /* bMS_VerdorCode */
    0,      /* bPad */
};
