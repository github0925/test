#!/bin/bash

PROJ="$1"

rm -rf ./t_loader_safety_merged.bin
cp ./t_loader.bin  ./t_loader_safety_merged.bin
dd if=../../../build_iar/$PROJ/Debug/Exe/$PROJ.bin >> ./t_loader_safety_merged.bin
./sign_tool/run_sign_safe ./t_loader_safety_merged.bin 0x4007800 0x4007800
cp ./t_loader_safety_merged.bin.safe.signed ./safety.img

