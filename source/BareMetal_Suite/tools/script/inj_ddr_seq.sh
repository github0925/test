#!/bin/bash

#DDR_INIT_SEQ_OFF=736

DDR_SEQ_PARSER=./tools/ddr_seq_parser/ddr_seq_parser

#gcc  -Idriver/ddr/dw_umctl2/inc/ -o $1.o -c $1

#objcopy -O binary $1.o $1.bin
$DDR_SEQ_PARSER if=$1 of=$1.bin
if [ $? -ne 0 ]; then
    echo -e "\e[1;33;41m Opps inj_ddr_seq form $1.bin fail !! \e[0m"
    exit 1
fi
dd of=$2 if=$1.bin bs=1024 seek=${DDR_INIT_SEQ_OFF}

rm $1.o $1.bin -rf
