#!/bin/bash

BIN=fw/ddr_squeezer_loader_debug.elf.bin
BIN_LOAD=ddr_squeezer_loader_usb.elf.bin
DDR_SEQ_PARSER=./tools/linux/ddr_seq_parser
BIN_INJ=./tools/linux/bin_injector
SIGNER=./tools/linux/atb_signer

if [ "$1" == "" -o "$1" == "help" ]; then
    echo "Usage: $0 ddr_init_seq sec_uart_num ap_uart_num"
    echo "      Take x9_evb board for example, the command is as followed"
    echo "      $0 ddr_script/x9_evb_lpddr4x_4266.c 10 9"
    exit 0
fi

if [ "$1" != "" ]; then
    cp $BIN $BIN_LOAD
    #gcc  -Iddr_script/headers/ -o $1.o -c $1
    #objcopy -O binary $1.o $1.bin
    $DDR_SEQ_PARSER if=$1 of=$1.bin
    $BIN_INJ if=$1.bin of=$BIN_LOAD seek=0x5c000
    rm $1.o $1.bin -rf

    if [ "$2" != "" ]; then
        $BIN_INJ of=$BIN_LOAD inj=0x5bfe0:0x01 inj=0x5bfe1:0x50 inj=0x5bfe2:0x41 inj=0x5bfe3:0x50
        $BIN_INJ of=$BIN_LOAD inj=0x5bfe4:$2 inj=0x5bfe5:0x00 inj=0x5bfe6:0x00 inj=0x5bfe7:0x00
        if [ "$3" != "" ]; then
            $BIN_INJ of=$BIN_LOAD inj=0x5bfe4:$2 inj=0x5bfe5:$3 inj=0x5bfe6:0x00 inj=0x5bfe7:0x00
        fi
    fi

    $SIGNER sign --sec_ver 1234 --pkg_ver 1 --rcp key=key/TestRSA1024_ossl.pem rot=1 --iib img=$BIN_LOAD to=0x140000 entry=0x140000 uuid=12345678ab --dgst sha256 --of $BIN_LOAD.signed.rsa1024

    echo
    echo "TestSetup Done."
    echo "Please put your board into USB boot mode (boot pin=4'b1000) then load this image using USB"
    echo "To run DDR_Squeezer, please type "memtester start size [loop=m] [core=n]" into uart terminal."
    echo "For example: memtester 0x40000000 0x100000000 loop=4 core=4"
    echo
    rm $BIN_LOAD -rf

fi
