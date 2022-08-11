#!/bin/bash

make clean
make CFG=kunlun_ap D=1  bin
if [ $? -ne 0 ]; then
    echo "$0: !! Opps, build_x9_evb_ap fail !!"
    exit 1
fi

make clean
make CFG=kunlun_sec D=1 PL=1 AP=1 DDR=1 DS=ddr_script/x9_evb_lpddr4x_4266.c bin
if [ $? -ne 0 ]; then
    echo "$0: !! Opps, build_x9_evb_sec fail !!"
    exit 1
fi

rm DDR_Squeezer_v$1 -rf
mkdir DDR_Squeezer_v$1
mkdir DDR_Squeezer_v$1/key
mkdir DDR_Squeezer_v$1/fw
mkdir DDR_Squeezer_v$1/tools
mkdir DDR_Squeezer_v$1/tools/linux

cp build/kunlun/sec/ddr_squeezer_loader_debug.elf.bin DDR_Squeezer_v$1/fw
cp ddr_script DDR_Squeezer_v$1/ -rf
cp tools/script/TestSetup.sh DDR_Squeezer_v$1/TestSetup.sh
cp tools/script/TestSetup.bat DDR_Squeezer_v$1/TestSetup.bat
cp tools/bin_injector/bin_injector DDR_Squeezer_v$1/tools/linux
cp tools/atb_signer DDR_Squeezer_v$1/tools/linux
cp tools/ddr_seq_parser/ddr_seq_parser  DDR_Squeezer_v$1/tools/linux
cp test/keys/TestRSA1024_ossl.pem DDR_Squeezer_v$1/key
cp tools/windows DDR_Squeezer_v$1/tools/ -rf
