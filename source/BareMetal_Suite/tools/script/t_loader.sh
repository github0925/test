#!/bin/bash

TOPDIR=$(pwd)
SIGN_CMD=${TOPDIR}/tools/atb_signer

if [ "$1" == "" -o "$1" == "help" ]; then
    echo "Usage: $0 safety_image_binary [debug]"
    exit 0
fi

if [ "$1" != "" ]; then
    if [ "$2" == "debug" ]; then
        tl_bin="build/kunlun/safe/safe_t_loader_debug.elf.bin"
    else
        tl_bin="build/kunlun/safe/safe_t_loader.elf.bin"
    fi

    if [ -f ${tl_bin} ]; then
        echo ""
    else
        make clean

        if [ "$2" == "debug" ]; then
            make CFG=t_loader BOARD=g9_ref D=1 bin
        else
            make CFG=t_loader BOARD=g9_ref bin
        fi
        if [ $? -ne 0 ]; then
            echo "$0: !! Opps, build_t_loader fail !!"
            exit 1
        fi
    fi

    rm -rf xxx.bin
    cp ${tl_bin} xxx.bin
    dd if=$1 >> xxx.bin
    ${SIGN_CMD} sign --rcp key=test/keys/TestRSA1024_ossl.pem rot=1 --iib img=xxx.bin to=0x4007800 entry=0x4007800 --dgst sha256 --of $1.tloader.signed.rsa1024

    rm xxx.bin
fi
