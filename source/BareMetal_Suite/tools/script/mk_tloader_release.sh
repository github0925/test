#!/bin/bash

make clean
make CFG=t_loader BOARD=g9_ref bin -j8
if [ $? -ne 0 ]; then
    echo "$0: !! Opps, build_t_loader fail !!"
    exit 1
fi
make clean
make CFG=t_loader BOARD=g9_ref D=1 bin -j8
if [ $? -ne 0 ]; then
    echo "$0: !! Opps, build_t_loader_debug fail !!"
    exit 1
fi
make clean
make CFG=test_app D=1 BOARD=g9_ref bin -j8
if [ $? -ne 0 ]; then
    echo "$0: !! Opps, build_test_app fail !!"
    exit 1
fi


rm tloader_v$1 -rf
mkdir tloader_v$1
mkdir tloader_v$1/test
mkdir tloader_v$1/test/keys
mkdir tloader_v$1/tools
mkdir tloader_v$1/tools/script
mkdir tloader_v$1/build
mkdir tloader_v$1/build/kunlun
mkdir tloader_v$1/build/kunlun/safe

cp build/kunlun/safe/safe_t_loader_debug.elf.bin tloader_v$1/build/kunlun/safe
cp build/kunlun/safe/safe_t_loader.elf.bin tloader_v$1/build/kunlun/safe
cp build//kunlun/safe/safe_test_app_debug.elf.bin tloader_v$1/build/kunlun/safe
cp build/ddr_fw tloader_v$1/build/ -rf

cp ddr_script tloader_v$1/ -rf
cp tools/atb_signer tloader_v$1/tools/
cp tools/ddr_seq_parser/ddr_seq_parser  tloader_v$1/tools/
cp test/keys/TestRSA1024_ossl.pem tloader_v$1/test/keys
cp tools/script/t_loader.sh tloader_v$1/tools/script
