#!/bin/bash
#\\192.168.1.13\rui.wang\ddr_squeezer\BareMetal_Suite\build\kunlun\sec\ddr_squeezer_loader_debug.elf.bin.signed.rsa1024  loadimage_name
if [ "$1" == "x9_sqz_ddr" -o "$1" == "all" -o "$1" == "" ]; then
    make clean
    make CFG=kunlun_ap_in_ddr D=1  IN_DDR_RUN=1 bin 
    if [ $? -ne 0 ]; then
        echo "$0: !! Opps, build_x9_suqeezer_ddr_ap fail !!"
        exit 1
    fi

    make clean
    make CFG=kunlun_sec IN_DDR_RUN=1  LOAD_BIN=ddr_squeezer_run_in_ddr_debug D=1 DDR=1 PL=1 AP=1  DS=ddr_script/x9_ref_ddr_init_lpddr4x_4266_8GB.c  bin
    if [ $? -ne 0 ]; then
        echo "$0: !! Opps, build_x9_suqeezer_ddr_sec fail !!"
        exit 1
    fi
fi

if [ "$1" == "x9_sqz_iram" -o "$1" == "all" -o "$1" == "" ]; then
    make clean
    make CFG=kunlun_ap D=1 bin
    if [ $? -ne 0 ]; then
        echo "$0: !! Opps, build_x9_suqeezer_iram_ap fail !!"
        exit 1
    fi

    make clean
    make CFG=kunlun_sec IN_DDR_RUN=0  D=1 PL=1 AP=1 DDR=1 DS=ddr_script/x9_ref_ddr_init_lpddr4x_4266_8GB.c bin
    if [ $? -ne 0 ]; then
        echo "$0: !! Opps, build_x9_suqeezer_iram_sec fail !!"
        exit 1
    fi
fi

if [ "$1" == "x9_evb" -o "$1" == "all" -o "$1" == "" ]; then
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
fi

if [ "$1" == "x9_ref" -o "$1" == "all" -o "$1" == "" ]; then
    make clean
    make CFG=kunlun_ap D=1  bin
    if [ $? -ne 0 ]; then
        echo "$0: !! Opps, build_x9_ref_ap fail !!"
        exit 1
    fi

    make clean
    make CFG=kunlun_sec D=1 PL=1 AP=1 DDR=1 DS=ddr_script/x9_ref_ddr_init_lpddr4x_4266_4GB.c BOARD=x9_ref bin
    if [ $? -ne 0 ]; then
        echo "$0: !! Opps, build_x9_ref_sec fail !!"
        exit 1
    fi

fi

if [ "$1" == "x9_ref_8g" -o "$1" == "all" -o "$1" == "" ]; then
    make clean
    make CFG=kunlun_ap D=1  bin
    if [ $? -ne 0 ]; then
        echo "$0: !! Opps, build_x9_ref_ap fail !!"
        exit 1
    fi
    make clean
    make CFG=kunlun_sec D=1 PL=1 AP=1 DDR=1 DS=ddr_script/x9_ref_ddr_init_lpddr4x_4266_8GB.c BOARD=x9_ref bin
    if [ $? -ne 0 ]; then
        echo "$0: !! Opps, build_x9_ref_8g_sec fail !!"
        exit 1
    fi
fi


if [ "$1" == "g9_ref" -o "$1" == "all" ]; then
    make clean
    make CFG=kunlun_ap D=1 BOARD=g9_ref bin
    if [ $? -ne 0 ]; then
        echo "$0: !! Opps, build_g9_ref_ap fail !!"
        exit 1
    fi

    make clean
    make CFG=kunlun_sec D=1 PL=1 AP=1 DDR=1 BOARD=g9_ref DS=ddr_script/g9_ref_lpddr4x_2133.c bin
    if [ $? -ne 0 ]; then
        echo "$0: !! Opps, build_g9_ref_sec fail !!"
        exit 1
    fi
    echo "build_g9_ref done"
fi

if [ "$1" == "ii4" -o "$1" == "all" ]; then
    make clean
    make CFG=kunlun_ap D=1 BOARD=g9_ref bin
    if [ $? -ne 0 ]; then
        echo "$0: !! Opps, build_g9_ref_ap fail !!"
        exit 1
    fi

    make clean
    make CFG=kunlun_sec D=1 PL=1 AP=1 DDR=1 BOARD=g9_ref DS=ddr_script/g9x_ii4_lpddr4_2133.c bin
    if [ $? -ne 0 ]; then
        echo "$0: !! Opps, build_g9_ref_sec fail !!"
        exit 1
    fi
    echo "build_g9_iib done"
fi
