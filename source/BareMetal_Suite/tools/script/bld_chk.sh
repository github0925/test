#!/bin/bash

make clean
make CFG=kunlun_sec D=1 bin
if [ $? -ne 0 ]; then
    echo "$0: !! Opps, build_kunlun_sec fail !!"
    exit 1
fi

make clean
make CFG=kunlun_ap D=1  bin
if [ $? -ne 0 ]; then
    echo "$0: !! Opps, build_kunlun_ap fail !!"
    exit 1
fi
