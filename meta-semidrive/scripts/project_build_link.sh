#!/bin/bash
#set -x
set -e

export SD_TOPDIR=`pwd`
AP_STR=""
CHIPVERSION=""
MACHINE_PROJECT=""
SD_BOOT_MODE="emmc_only"
SD_AACH64_GCC_DIR="$PATH:${SD_TOPDIR}/source/toolchain/gcc_linaro/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-elf/bin"
export PATH=$PATH:${SD_TOPDIR}/source/toolchain/gcc_linaro/gcc-arm-none-eabi-7.3.1/bin:${SD_AACH64_GCC_DIR}:${SD_TOPDIR}/source/toolchain/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin
SD_KERNEL_DIR="${SD_TOPDIR}/source/linux"
#SD_KERNEL5.10_DIR="${SD_TOPDIR}/source/linux5.10"

function add_var_to_project_config
{
    #re create .project.conf at SD_TOPDIR
    DEPLOYDIR=${SD_TOPDIR}/out/${MACHINE_DEPLOYDIR}/binary
    SD_BUILD_OBJDIR=${SD_TOPDIR}/out/${MACHINE_DEPLOYDIR}/build_object
    SD_DEPLOY_IMG=${SD_TOPDIR}/out/${MACHINE_DEPLOYDIR}/images
    if [ ! -d ${DEPLOYDIR} ];then mkdir -p ${DEPLOYDIR};fi
    if [ ! -d ${SD_BUILD_OBJDIR} ];then mkdir -p ${SD_BUILD_OBJDIR};fi
    if [ ! -d ${SD_DEPLOY_IMG} ];then mkdir -p ${SD_DEPLOY_IMG};fi

    SD_BR_OUT_PATH=${SD_BUILD_OBJDIR}/${MACHINE_NAME}_buildroot
    SD_BR_OUT_PATH2=${SD_BUILD_OBJDIR}/${MACHINE_NAME2}_buildroot

    SD_KERNEL_OUT=${SD_BUILD_OBJDIR}/${MACHINE_NAME}_linux
    SD_KERNEL_OUT2=${SD_BUILD_OBJDIR}/${MACHINE_NAME2}_linux

    SD_UBOOT_OUT=${SD_BUILD_OBJDIR}/${MACHINE_NAME}_uboot
    SD_UBOOT_OUT2=${SD_BUILD_OBJDIR}/${MACHINE_NAME2}_uboot

    #begin to write to .project.conf
    set | grep -E "SD_*|MACHINE_*|PROJECT_*" > .project.conf
    
    #rm unuse line by key word
    sed -i '/\$/d' .project.conf
    sed -i '/set/d' .project.conf
    sed -i '/echo/d' .project.conf

    #move MACHINE_PROJECT CHIPVERSION SD_BOOT_MODE... to the top in .project.conf
    sed -i "1i SD_AACH64_GCC_DIR=${SD_AACH64_GCC_DIR}" .project.conf
    sed -i "1i SD_BOOT_MODE=${SD_BOOT_MODE}" .project.conf
    sed -i '/MACHINE_PROJECT=/d' .project.conf
    sed -i "1i MACHINE_PROJECT=${MACHINE_PROJECT}" .project.conf
    sed -i "1i CHIPVERSION=${CHIPVERSION}" .project.conf
    #add warnning at the top
    sed -i "1i ###goto ${PROJECT_CONFIG} to change your config" .project.conf
    sed -i "1i ###dont change this file" .project.conf

    #add some useful var in .project.conf
    echo "BAREMETAL_TGT=${BAREMETAL_TGT}" >> .project.conf
    echo "PRELOAD_RES_SIZE=${PRELOAD_RES_SIZE}" >> .project.conf
    echo "DEPLOYDIR=${DEPLOYDIR}" >> .project.conf
    echo "SD_BUILD_OBJDIR=${SD_BUILD_OBJDIR}" >> .project.conf
    echo "SD_DEPLOY_IMG=${SD_DEPLOY_IMG}" >> .project.conf
    echo "SD_BR_OUT_PATH=${SD_BR_OUT_PATH}" >> .project.conf
    echo "SD_BR_OUT_PATH2=${SD_BR_OUT_PATH2}" >> .project.conf
    echo "SD_KERNEL_OUT=${SD_KERNEL_OUT}" >> .project.conf
    echo "SD_KERNEL_OUT2=${SD_KERNEL_OUT2}" >> .project.conf
    echo "SD_UBOOT_OUT=${SD_UBOOT_OUT}" >> .project.conf
    echo "SD_UBOOT_OUT2=${SD_UBOOT_OUT2}" >> .project.conf
    echo "SD_KERNEL_DIR=${SD_KERNEL_DIR}" >> .project.conf
}

function do_config() {
    (
	if [ "x$CHIPVERSION" == "x" ] || [ "x$MACHINE_PROJECT" == "x" ];then
		echo "need to config CHIPVERSION and MACHINE_PROJECT"
		usage
		if [[ "$0" == "-bash" ]];then return 0; else exit 1; fi
	fi

    SD_DIR_PROJECT_CONFIG="${SD_TOPDIR}/source/chipcfg/generate/${CHIPVERSION}/projects/${MACHINE_PROJECT}"
    PROJECT_CONFIG="${SD_DIR_PROJECT_CONFIG}/project.conf"

    if [ -f ${PROJECT_CONFIG} ];then
        
        if [ -e project_chipcfg_link ];then rm project_chipcfg_link; fi
        ln -s ${SD_DIR_PROJECT_CONFIG} project_chipcfg_link

        source ${PROJECT_CONFIG}
        add_var_to_project_config
        
    else
        echo "can not find ${PROJECT_CONFIG},please check if it exists"
    fi

	if [ -h ${SD_TOPDIR}/meta-semidrive/scripts/project_conf_link ];then
		rm -f ${SD_TOPDIR}/meta-semidrive/scripts/project_conf_link
	fi
	ln -s ${SD_DIR_PROJECT_CONFIG}/project.conf \
    meta-semidrive/scripts/project_conf_link

    if [ -h ${SD_TOPDIR}/meta-semidrive/scripts/pack_cfg.sh ];then
		rm -f ${SD_TOPDIR}/meta-semidrive/scripts/pack_cfg.sh
	fi
	ln -s ../../source/chipcfg/generate/${CHIPVERSION}/projects/${MACHINE_PROJECT}/pack_cfg/pack_cfg.sh \
    meta-semidrive/scripts/pack_cfg.sh

	#ln -s ${SD_DIR_PROJECT_CONFIG} ${MACHINE_PROJECT}
    (
    cd meta-semidrive/scripts/
    if [ -h ${MACHINE_PROJECT} ];then rm ${MACHINE_PROJECT};fi
    ln -s ../../source/chipcfg/generate/${CHIPVERSION}/projects/${MACHINE_PROJECT}/pack_cfg/  ${MACHINE_PROJECT}
    )
    echo "update ssdk'd defconfig"
    if [ -f ${SD_TOPDIR}/source/ssdk/boards/d9_safety_ref/configs/${CHIPVERSION}_defconfig ]; then
        cp -f ${SD_TOPDIR}/source/ssdk/boards/d9_safety_ref/configs/${CHIPVERSION}_defconfig  ${SD_TOPDIR}/source/ssdk/boards/d9_safety_ref/defconfig;
    fi

    if [ -f ${SD_TOPDIR}/source/ssdk/boards/d9_safety_ref/configs/config_${CHIPVERSION}.h ]; then
        cp -f ${SD_TOPDIR}/source/ssdk/boards/d9_safety_ref/configs/config_${CHIPVERSION}.h  ${SD_TOPDIR}/source/ssdk/boards/d9_safety_ref/config.h;
    fi

    if [ -f ${SD_TOPDIR}/source/ssdk/boards/d9_secure_ref/configs/${CHIPVERSION}_defconfig ]; then
        cp -f ${SD_TOPDIR}/source/ssdk/boards/d9_secure_ref/configs/${CHIPVERSION}_defconfig  ${SD_TOPDIR}/source/ssdk/boards/d9_secure_ref/defconfig;
    fi

    if [ -f ${SD_TOPDIR}/source/ssdk/boards/d9_secure_ref/configs/config_${CHIPVERSION}.h ]; then
        cp -f ${SD_TOPDIR}/source/ssdk/boards/d9_secure_ref/configs/config_${CHIPVERSION}.h  ${SD_TOPDIR}/source/ssdk/boards/d9_secure_ref/config.h;
    fi
    )
}

function usage() {
    (
        echo "==========================================================================="
        echo "=  ####   ######  #    #    ###   #####   #####     #    #    #  ######  =="
        echo "= #       #       ##  ##     #    #    #  #    #    #    #    #  #       =="
        echo "=  ####   #####   # ## #     #    #    #  #    #    #    #    #  #####   =="
        echo "=      #  #       #    #     #    #    #  #####     #    #    #  #       =="
        echo "= #    #  #       #    #     #    #    #  #   #     #     #  #   #       =="
        echo "=  ####   ######  #    #    ###   #####   #    #    #      ##    ######  =="
        echo "==========================================================================="
        echo "==========================================================================="

        echo -e "============Usage1:======================================================"
        echo -e "*Step 1: \033[0;31;1m./build.sh config\033[0m                                              *"
        echo -e "*Step 2: (select chip board and boot-mode)                              *"
        echo -e "*Step 3: \033[0;31;1m./build.sh\033[0m                                                     *"
        echo -e "*===========Usage2:=====================================================*"
        echo -e "*Step 1: \033[0;31;1m./build.sh config [chip] [board] [boot-mode]\033[0m                   *"
        echo -e "*Step 2: \033[0;31;1m./build.sh [module]\033[0m                                            *"
        echo -e "*  [chip]: d9, d9lite, d9plus                                           *"
        echo -e "*  [board]: d9plus_ref,d9_ref,d9lite_ref                                *"
        echo -e "*  [boot-mode]: emmc_only, ospi1, sdcard                                *"
        echo -e "*  [module]:\033[0;31;1mbm,lk,safety,linux,uboot,ssdk,buildroot,ubuntu,pack\033[0m         *"
        echo -e "*===========Example1:===================================================*"
        echo -e "*Step 1: \033[0;31;1m./build.sh config d9 d9_ref emmc_only\033[0m                          *"
        echo -e "*Step 2: \033[0;31;1m./build.sh \033[0m  compile and then pack                             *"
        echo -e "*===========Example2:===================================================*"
        echo -e "*Step 1: \033[0;31;1m./build.sh config\033[0m                                              *"
        echo -e "*Step 2: (select chip board and boot-mode)                              *"
        echo -e "*Step 3: \033[0;31;1m./build.sh lk\033[0m compile lk only                                  *"
        echo -e "*Step 4: \033[0;31;1m./build.sh bm\033[0m compile baremetal only                           *"
        echo -e "*Step 5: \033[0;31;1m./build.sh linux\033[0m compile kernel only                           *"
        echo -e "========================================================================="
    )
}

function list_choose() {
    local i=1
    local choice
    list_str=$*
    echo "${FUNCNAME[1]}"
    for choice in $list_str; do
        echo "    $i: $choice"
        i=$(($i+1))
    done
    echo -n "choose: "
}

function chip_choose() {
    list_str=`cd source/chipcfg/generate/; ls -d */ | sed "s/\///g"`
    list_choose  $list_str

    read answer
    if ! (echo -n $answer | grep -q -e "^[0-9][0-9]*$"); then echo "${FUNCNAME[0]} error";return -1;fi

    list_str=($list_str)
    CHIPVERSION=${list_str[$answer-1]}
    if [ "x$CHIPVERSION" == "x" ];then echo "${FUNCNAME[0]} error";return -1;fi
    echo -e "\033[0;31;1mselect CHIPVERSION=$CHIPVERSION\033[0m"
}

function machine_project_choose() {
    list_str=`cd source/chipcfg/generate/${CHIPVERSION}/projects/; ls -d */ | sed "s/\///g"`
    list_choose  $list_str

    read answer
    if ! (echo -n $answer | grep -q -e "^[0-9][0-9]*$"); then echo "${FUNCNAME[0]} error";return -1;fi

    list_str=($list_str)
    MACHINE_PROJECT=${list_str[$answer-1]}
    if [ "x$MACHINE_PROJECT" == "x" ];then echo "${FUNCNAME[0]} error";return -1;fi
    echo -e "\033[0;31;1mselect MACHINE_PROJECT=$MACHINE_PROJECT\033[0m"
}

function boot_mode_choose() {
    list_str="ospi1 emmc_only sdcard"
    list_choose  $list_str

    read answer
    if ! (echo -n $answer | grep -q -e "^[0-9][0-9]*$"); then echo "${FUNCNAME[0]} error";return -1;fi

    list_str=($list_str)
    SD_BOOT_MODE=${list_str[$answer-1]}
    if [ "x$SD_BOOT_MODE" == "x" ];then echo "${FUNCNAME[0]} error";return -1;fi
    echo -e "\033[0;31;1mselect SD_BOOT_MODE=$SD_BOOT_MODE\033[0m"
}

function linuxversion_choose() {
    list_str=    list_str=`cd source/; ls -d */ | sed "s/\///g"|grep linux`
    list_choose  $list_str

    read answer
    if ! (echo -n $answer | grep -q -e "^[0-9][0-9]*$"); then echo "${FUNCNAME[0]} error";return -1;fi

    list_str=($list_str)
    SEL_KERNEL_DIR=${list_str[$answer-1]}
    if [ "x$SEL_KERNEL_DIR" == "x" ];then echo "${FUNCNAME[0]} error";return -1;fi

    SD_KERNEL_DIR="${SD_TOPDIR}/source/${SEL_KERNEL_DIR}"
    echo -e "\033[0;31;1mselect SD_KERNEL_DIR=$SD_KERNEL_DIR\033[0m"

    export SD_KERNEL_DIR
}

case "$1" in
    -h)
        usage
        if [[ "$0" == "-bash" ]];then return 0; else exit 1; fi
        ;;
    config)
        if [ $# -ge 3 ];then
            CHIPVERSION=$2
            MACHINE_PROJECT=$3
            SD_BOOT_MODE=$4
            do_config
        elif [ $# -eq 1 ];then
		    chip_choose
            machine_project_choose
            boot_mode_choose
            linuxversion_choose
            do_config
        fi
        usage

        #rm cur_build.log at config state everytime
        if [ -e ${SD_TOPDIR}/cur_build.log ];then rm -f cur_build.log;fi
        if [[ "$0" == "-bash" ]];then return 0; else exit 1; fi
        ;;
esac

function set_project_config()
{
    if [ -f .project.conf ];then
        source .project.conf
    else
        echo "can not find .project.conf,need to config first"
        usage
        if [[ "$0" == "-bash" ]];then return 0; else exit 1; fi
    fi

    if [ "x$CHIPVERSION" == "x" ] || [ "x$MACHINE_PROJECT" == "x" ];then
		echo "set_project_config: need to config CHIPVERSION and MACHINE_PROJECT"
		usage
		if [[ "$0" == "-bash" ]];then return 0; else exit 1; fi
	fi

    PROJECT_CONFIG="${SD_DIR_PROJECT_CONFIG}/project.conf"
    if [ -f  ${PROJECT_CONFIG} ];then
        source ${PROJECT_CONFIG}
    else
        echo "CAN NOT FOUND ${PROJECT_CONFIG}"
        usage
        if [[ "$0" == "-bash" ]];then return 0; else exit 1; fi
    fi
}

function boot_mode_handling()
{
    # in ospi boot mode,we need to set next variables as empty
    if [ "xospi1" == "x$SD_BOOT_MODE" ]; then
        BAREMETAL_TGT=
        PRELOAD_RES_SIZE=
        MACHINE_SAFETYARG=${SD_PROJECT_BASE_CFG}
        MACHINE_DLOADERARG=${SD_PROJECT_BASE_CFG}
    fi
}

set_project_config
boot_mode_handling


EXTRA_OEMAKE="-j${SD_KERN_JLEVEL}"


function print_error()
{
    echo -e "\033[47;31m found ERROR: $*\033[0m"
}

function bbplain()
{
    echo -e "\033[47;31m $*\033[0m"
    if [ ! -e ${SD_TOPDIR}/cur_build.log ];then touch cur_build.log;fi
    echo -e "$*" >> ${SD_TOPDIR}/cur_build.log
}

function do_compile_lk() {
	(
	cd ${SD_TOPDIR}/source/lk_customize/
    if [ "x${MACHINE_SPL}" != "x" ]; then
	bbplain "SPL compile CMD......[ make ${EXTRA_OEMAKE} ${MACHINE_SPLARG} VERIFIED_BOOT=${VERIFIED_BOOT} ${MACHINE_SPL} ]"
        make ${EXTRA_OEMAKE} ${MACHINE_SPLARG} VERIFIED_BOOT=${VERIFIED_BOOT} ${MACHINE_SPL}
    fi
    if [ "x${MACHINE_SSYSTEM}" != "x" ]; then
	bbplain "SSYSTEM compile CMD......[ make ${EXTRA_OEMAKE} ${MACHINE_SSYSTEMARG} VERIFIED_BOOT=${VERIFIED_BOOT} ${MACHINE_SSYSTEM} ]"
        make ${EXTRA_OEMAKE} ${MACHINE_SSYSTEMARG} VERIFIED_BOOT=${VERIFIED_BOOT} ${MACHINE_SSYSTEM}
    fi
    if [ "x${MACHINE_DLOADER}" != "x" ]; then
	bbplain "DLOADER compile CMD......[ make ${EXTRA_OEMAKE} ${MACHINE_DLOADERARG} VERIFIED_BOOT=${VERIFIED_BOOT} ${MACHINE_DLOADER} ]"
        make ${EXTRA_OEMAKE} ${MACHINE_DLOADERARG} VERIFIED_BOOT=${VERIFIED_BOOT} ${MACHINE_DLOADER}
    fi
    if [ "x${MACHINE_PRELOADER}" != "x" ]; then
	bbplain "PRELOADER compile CMD......[ make ${EXTRA_OEMAKE} ${MACHINE_PRELOADERARG} VERIFIED_BOOT=${VERIFIED_BOOT} ${MACHINE_PRELOADER} ]"
        make ${EXTRA_OEMAKE} ${MACHINE_PRELOADERARG} VERIFIED_BOOT=${VERIFIED_BOOT} ${MACHINE_PRELOADER}
    fi
	if [ "x${MACHINE_PRELOADER2}" != "x" ]; then
	bbplain "PRELOADER compile CMD......[ make ${EXTRA_OEMAKE} ${MACHINE_PRELOADERARG2} VERIFIED_BOOT=${VERIFIED_BOOT} ${MACHINE_PRELOADER2} ]"
        make ${EXTRA_OEMAKE} ${MACHINE_PRELOADERARG2}  VERIFIED_BOOT=${VERIFIED_BOOT} ${MACHINE_PRELOADER2}
    fi
    if [ "x${MACHINE_BOOTLOADER}" != "x" ]; then
	bbplain "BOOTLOADER compile CMD......[ make ${EXTRA_OEMAKE} ${MACHINE_BOOTLOADERARG} VERIFIED_BOOT=${VERIFIED_BOOT} ${MACHINE_BOOTLOADER} ]"
        make ${EXTRA_OEMAKE} ${MACHINE_BOOTLOADERARG} VERIFIED_BOOT=${VERIFIED_BOOT} ${MACHINE_BOOTLOADER}
    fi
	if [ "x${MACHINE_BOOTLOADER2}" != "x" ]; then
	bbplain "BOOTLOADER compile CMD......[ make ${EXTRA_OEMAKE} ${MACHINE_BOOTLOADERARG2} VERIFIED_BOOT=${VERIFIED_BOOT} ${MACHINE_BOOTLOADER2} ]"
        make ${EXTRA_OEMAKE} ${MACHINE_BOOTLOADERARG2} VERIFIED_BOOT=${VERIFIED_BOOT} ${MACHINE_BOOTLOADER2}
    fi
    if [ "x${MACHINE_MP}" != "x" ]; then
	bbplain "MP compile CMD......[ make ${EXTRA_OEMAKE} ${MACHINE_MP} SUPPORT_BOARD_DIAG=${SUPPORT_BOARD_DIAG} VERIFIED_BOOT=${VERIFIED_BOOT} ]"
        make ${EXTRA_OEMAKE} ${MACHINE_MP} SUPPORT_BOARD_DIAG=${SUPPORT_BOARD_DIAG} VERIFIED_BOOT=${VERIFIED_BOOT}
    fi
	)
}
do_deploy_lk() {
    if [ x$1 != x ] ; then
        bbplain "-----$1------"
    fi
    (
	cd ${SD_TOPDIR}/source/lk_customize/
    if [ "x${MACHINE_SSYSTEM}" != "x" ]; then
		bbplain "SSYSTEM install CMD......[ install -m 0644 build-${MACHINE_SSYSTEM}${PROJECT_BUILDDIR_POSTFIX}/lk.bin ${DEPLOYDIR}/ssystem.bin ]"
        install -m 0644 build-${MACHINE_SSYSTEM}${PROJECT_BUILDDIR_POSTFIX}/lk.bin ${DEPLOYDIR}/ssystem.bin
    fi
    if [ "x${MACHINE_SPL}" != "x" ]; then
		bbplain "SPL install CMD......[ install -m 0644 build-${MACHINE_SPL}${PROJECT_BUILDDIR_POSTFIX}/lk.bin ${DEPLOYDIR}/spl.bin ]"
        install -m 0644 build-${MACHINE_SPL}${PROJECT_BUILDDIR_POSTFIX}/lk.bin ${DEPLOYDIR}/spl.bin
	install -m 0644 build-${MACHINE_SPL}${PROJECT_BUILDDIR_POSTFIX}/ddr_fw.bin ${DEPLOYDIR}/ddr_fw.bin
        install -m 0644 build-${MACHINE_SPL}${PROJECT_BUILDDIR_POSTFIX}/ddr_init_seq.bin ${DEPLOYDIR}/ddr_init_seq.bin
    fi
    if [ "x${MACHINE_DLOADER}" != "x" ]; then
		bbplain "DLOADER install CMD......[ install -m 0644 build-${MACHINE_DLOADER}${PROJECT_BUILDDIR_POSTFIX}/lk.bin ${DEPLOYDIR}/dloader.bin ]"
        install -m 0644 build-${MACHINE_DLOADER}${PROJECT_BUILDDIR_POSTFIX}/lk.bin ${DEPLOYDIR}/dloader.bin
    fi
    if [ "x${MACHINE_PRELOADER}" != "x" ]; then
		bbplain "PRELOADER install CMD......[ install -m 0644 build-${MACHINE_PRELOADER}${PROJECT_BUILDDIR_POSTFIX}/lk.bin ${DEPLOYDIR}/preloader.bin ]"
        install -m 0644 build-${MACHINE_PRELOADER}${PROJECT_BUILDDIR_POSTFIX}/lk.bin ${DEPLOYDIR}/preloader.bin
    fi
    if [ "x${MACHINE_BOOTLOADER}" != "x" ]; then
		bbplain "BOOTLOADER install CMD......[ install -m 0644 build-${MACHINE_BOOTLOADER}${PROJECT_BUILDDIR_POSTFIX}/lk.bin ${DEPLOYDIR}/bootloader.bin ]"
        install -m 0644 build-${MACHINE_BOOTLOADER}${PROJECT_BUILDDIR_POSTFIX}/lk.bin ${DEPLOYDIR}/bootloader.bin
    fi
    if [ "x${MACHINE_PRELOADER2}" != "x" ]; then
		bbplain "PRELOADER install CMD......[ install -m 0644 build-${MACHINE_PRELOADER2}${PROJECT_BUILDDIR_POSTFIX}/lk.bin ${DEPLOYDIR}/preloader2.bin ]"
        install -m 0644 build-${MACHINE_PRELOADER2}${PROJECT_BUILDDIR_POSTFIX}/lk.bin ${DEPLOYDIR}/preloader2.bin
    fi
    if [ "x${MACHINE_BOOTLOADER2}" != "x" ]; then
		bbplain "BOOTLOADER install CMD......[ install -m 0644 build-${MACHINE_BOOTLOADER2}${PROJECT_BUILDDIR_POSTFIX}/lk.bin ${DEPLOYDIR}/bootloader2.bin ]"
        install -m 0644 build-${MACHINE_BOOTLOADER2}${PROJECT_BUILDDIR_POSTFIX}/lk.bin ${DEPLOYDIR}/bootloader2.bin
    fi
    if [ -e build-${MACHINE_MP} ]; then
		bbplain "MP install CMD......[ install -m 0644 build-${MACHINE_MP}${PROJECT_BUILDDIR_POSTFIX}/sdpe-enc.bin ${DEPLOYDIR}/sdpe.bin ]"
        openssl enc -aes-128-cbc -in build-${MACHINE_MP}${PROJECT_BUILDDIR_POSTFIX}/lk.bin -out  build-${MACHINE_MP}/sdpe-enc.bin -K 9fd7d6be16e1cd6ee2db101c7cbb2fbe -iv ca69d8879bcb539ef025c195f3ad1517
        install -m 0644 build-${MACHINE_MP}${PROJECT_BUILDDIR_POSTFIX}/sdpe-enc.bin ${DEPLOYDIR}/sdpe.bin
        install -m 0644 build-${MACHINE_MP}${PROJECT_BUILDDIR_POSTFIX}/sdpe_cfg.bin ${DEPLOYDIR}/sdpe_cfg.bin
    fi
	)
}

function do_compile_safety() {
    if [ "x$SD_BOOT_MODE" == "xemmc_only" ];then
		bptfile="global_emmc_only.bpt"
    elif [ "x$SD_BOOT_MODE" == "xsdcard" ];then
        bptfile="global_sdcard_only.bpt"
    else
        bptfile="ospi.bpt"
    fi

    BSPDIR=${SD_TOPDIR}
    BPTTOOL=${BSPDIR}/meta-semidrive/scripts/bpttool
    BPTFILE=${BSPDIR}/meta-semidrive/scripts/${MACHINE_NAME}/${bptfile}
    BPT_RES_PARITION_SZ=${PRELOAD_RES_SIZE}
    echo "default PRELOAD_RES_SIZE:"${BPT_RES_PARITION_SZ}
    if [ -n ${PRELOAD_RES_SIZE} -a -f ${BPTFILE} ];then
        res_size=`${BPTTOOL} query_partition --input ${BPTFILE} --label "res" --type size`
        echo "res partition size:"${res_size}
        BPT_RES_PARITION_SZ=${res_size}
        echo "BPT_RES_PARITION_SZ:"${BPT_RES_PARITION_SZ}
    fi
    (
    cd ${SD_TOPDIR}/source/lk_safety/
    FreeRTOS_DIR=${PWD}/../../source/freertos/FreeRTOS
    bbplain "SAFETY compile CMD......[     FreeRTOS_ROOT=${FreeRTOS_DIR} make ${EXTRA_OEMAKE} ${MACHINE_SAFETYARG} PRELOAD_RES_SIZE=${BPT_RES_PARITION_SZ} ${MACHINE_SAFETY} SUPPORT_BOARD_DIAG=${SUPPORT_BOARD_DIAG} VERIFIED_BOOT=${VERIFIED_BOOT} ]"

    FreeRTOS_ROOT=${FreeRTOS_DIR} make ${EXTRA_OEMAKE} ${MACHINE_SAFETYARG} PRELOAD_RES_SIZE=${BPT_RES_PARITION_SZ} ${MACHINE_SAFETY} SUPPORT_BOARD_DIAG=${SUPPORT_BOARD_DIAG} VERIFIED_BOOT=${VERIFIED_BOOT}
    bbplain "SAFETY compile CMD......[ FreeRTOS_ROOT=${FreeRTOS_DIR} make ${EXTRA_OEMAKE} ${MACHINE_OSPIHANDOVER}  ${MACHINE_OSPIHANDOVERARG} ]"
    FreeRTOS_ROOT=${FreeRTOS_DIR} make ${EXTRA_OEMAKE} ${MACHINE_OSPIHANDOVER}  ${MACHINE_OSPIHANDOVERARG}
    bbplain "SAFETY compile CMD......[ FreeRTOS_ROOT=${FreeRTOS_DIR} make ${EXTRA_OEMAKE} ${MACHINE_SAFETYARG} PRELOAD_RES_SIZE=${BPT_RES_PARITION_SZ} dil2 VERIFIED_BOOT=${VERIFIED_BOOT} ]"
    FreeRTOS_ROOT=${FreeRTOS_DIR} make ${EXTRA_OEMAKE} ${MACHINE_SAFETYARG} PRELOAD_RES_SIZE=${BPT_RES_PARITION_SZ} dil2 VERIFIED_BOOT=${VERIFIED_BOOT}
	)
}

do_deploy_safety() {
	(
    cd ${SD_TOPDIR}/source/lk_safety/
    bbplain "SAFETY install CMD......[ install -m 0644 build-${MACHINE_SAFETY}${PROJECT_BUILDDIR_POSTFIX}/${MACHINE_SAFETY}.bin ${DEPLOYDIR}/safety.bin ]"
    install -m 0644 build-${MACHINE_SAFETY}${PROJECT_BUILDDIR_POSTFIX}/${MACHINE_SAFETY}.bin ${DEPLOYDIR}/safety.bin
    bbplain "SAFETY install CMD......[ install -m 0644 build-${MACHINE_OSPIHANDOVER}${PROJECT_BUILDDIR_POSTFIX}/${MACHINE_OSPIHANDOVER}.bin ${DEPLOYDIR}/ospihandover.bin ]"
    install -m 0644 build-${MACHINE_OSPIHANDOVER}${PROJECT_BUILDDIR_POSTFIX}/${MACHINE_OSPIHANDOVER}.bin ${DEPLOYDIR}/ospihandover.bin
	
    bbplain "SAFETY install CMD......[ install -m 0644 build-dil2${PROJECT_BUILDDIR_POSTFIX}/dil2.bin ${DEPLOYDIR}/dil2.bin ]"
    install -m 0644 build-dil2${PROJECT_BUILDDIR_POSTFIX}/dil2.bin ${DEPLOYDIR}/dil2.bin
    install -m 0644 build-dil2${PROJECT_BUILDDIR_POSTFIX}/dil2.bin ${DEPLOYDIR}/dil2-unsigned.bin
    
    safety_sign_bin=`find build-${MACHINE_SAFETY}${PROJECT_BUILDDIR_POSTFIX} -type f -name *.safe.signed`
    if [ -f $safety_sign_bin ];then
		bbplain "SAFETY install CMD......[ install -m 0644 $safety_sign_bin ${DEPLOYDIR}/safety.bin.signed ]"
        install -m 0644 $safety_sign_bin ${DEPLOYDIR}/safety.bin.signed
    fi
    ospihandover_sign_bin=`find build-${MACHINE_OSPIHANDOVER}${PROJECT_BUILDDIR_POSTFIX} -type f -name *.safe.signed`
    if [ -f $ospihandover_sign_bin ];then
		bbplain "SAFETY install CMD......[ install -m 0644 $ospihandover_sign_bin ${DEPLOYDIR}/ospihandover.bin.signed ]"
        install -m 0644 $ospihandover_sign_bin ${DEPLOYDIR}/ospihandover.bin.signed
    fi
	)
}

function do_compile_bm() {
    if [ "x$SD_BOOT_MODE" == "xemmc_only" ];then
		bptfile="global_emmc_only.bpt"
    elif [ "x$SD_BOOT_MODE" == "xsdcard" ];then
        bptfile="global_sdcard_only.bpt"
    else
        bptfile="ospi.bpt"
    fi

    BSPDIR=${SD_TOPDIR}
    BPTTOOL=${BSPDIR}/meta-semidrive/scripts/bpttool
    BPTFILE=${BSPDIR}/meta-semidrive/scripts/${MACHINE_NAME}/${bptfile}
    BPT_RES_PARITION_SZ=${PRELOAD_RES_SIZE}
    echo "default PRELOAD_RES_SIZE:"${BPT_RES_PARITION_SZ}
    if [ -n ${PRELOAD_RES_SIZE} -a -f ${BPTFILE} ];then
        res_size=`${BPTTOOL} query_partition --input ${BPTFILE} --label "res" --type size`
        echo "res partition size:"${res_size}
        BPT_RES_PARITION_SZ=${res_size}
        echo "BPT_RES_PARITION_SZ:"${BPT_RES_PARITION_SZ}
    fi
	(
	cd ${SD_TOPDIR}/source/BareMetal_Suite/
    make CFG=t_loader clean -j2
    if [ "x${BAREMETAL_TGT}" != "x" ];then
        bbplain "BAREMETAL compile CMD......[ make CFG=t_loader unified_boot ${MACHINE_BAREMETALARG} PRELOAD_RES_SIZE=${BPT_RES_PARITION_SZ}  TGT=${BAREMETAL_TGT} VERIFIED_BOOT=${VERIFIED_BOOT} ]"
        make CFG=t_loader unified_boot ${MACHINE_BAREMETALARG} PRELOAD_RES_SIZE=${BPT_RES_PARITION_SZ}  TGT=${BAREMETAL_TGT} VERIFIED_BOOT=${VERIFIED_BOOT}
    else
        bbplain "BAREMETAL compile CMD......[ make CFG=t_loader unified_boot ${MACHINE_BAREMETALARG} VERIFIED_BOOT=${VERIFIED_BOOT} ]"
        make CFG=t_loader unified_boot ${MACHINE_BAREMETALARG} VERIFIED_BOOT=${VERIFIED_BOOT}
    fi
)
}

do_deploy_bm() {
    (
    cd ${SD_TOPDIR}/source/BareMetal_Suite/

    if [ x${BAREMETAL_TGT} != x"" ];then
        bbplain "BAREMETAL install CMD......[ install -m 0644 build/kunlun/${BAREMETAL_TGT}/${BAREMETAL_TGT}_t_loader.elf.bin ${DEPLOYDIR}/dil.bin ]"
        install -m 0644 build/kunlun/${BAREMETAL_TGT}/${BAREMETAL_TGT}_t_loader.elf.bin ${DEPLOYDIR}/dil.bin
    else
        bbplain "BAREMETAL install CMD......[ install -m 0644 build/kunlun/safe/safe_t_loader.elf.bin ${DEPLOYDIR}/dil.bin ]"
        install -m 0644 build/kunlun/safe/safe_t_loader.elf.bin ${DEPLOYDIR}/dil.bin
    fi
    )
}

export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-
export LOCALVERSION=""
KERNEL_VERSION="4.14.61"

function do_compile_linux()
{
(  
    echo "Building linux kernel "

    cd ${SD_KERNEL_DIR}
    KERNEL_VERSION=`make -s kernelversion -C ./`
    
    # Image is arm architecture specific target
    local arch_target=""

    if [ ! "x$MACHINE_NAME" = "x" ] ;then
        if [ ! -d ${SD_KERNEL_OUT} ] ; then
            mkdir -p ${SD_KERNEL_OUT}
        fi
            arch_target="Image Image.gz ${SD_KERN_DEVICETREE}"

        if [ ! -f ${SD_KERNEL_OUT}/.config ] ; then
            printf "\n\033[0;31;1mUsing default config ${SD_KERN_DEFCONF} ...\033[0m\n\n"
            make ARCH=${ARCH} ${SD_KERN_DEFCONF} O=${SD_KERNEL_OUT}
        fi

        make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -j${SD_KERN_JLEVEL} ${arch_target} modules O=${SD_KERNEL_OUT}
    else
        echo "MACHINE_NAME is null" 
    fi
    
    if [ ! "x$MACHINE_NAME2" = "x" ] ;then
        if [ ! -d ${SD_KERNEL_OUT2} ] ; then
            mkdir -p ${SD_KERNEL_OUT2}
        fi
            arch_target="Image Image.gz ${SD_KERN_DEVICETREE2}"

        if [ ! -f ${SD_KERNEL_OUT2}/.config ] ; then
            printf "\n\033[0;31;1mUsing default config ${SD_KERN_DEFCONF2} ...\033[0m\n\n"
            make ARCH=${ARCH} ${SD_KERN_DEFCONF2} O=${SD_KERNEL_OUT2}
        fi

        make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -j${SD_KERN_JLEVEL} ${arch_target} modules O=${SD_KERNEL_OUT2}
    else
        echo "MACHINE_NAME2 is null"   
    fi
)
}

function do_deploy_linux5.10()
{
    #cd ${SD_KERNEL5.10_DIR}
	cd ${SD_TOPDIR}/source/linux5.10
    KERNEL_VERSION=`make -s kernelversion -C ./`
(  
    echo "deploy linux kernel5.10 "

    # Image is arm architecture specific target
    local arch_target=""

    if [ ! "x$MACHINE_NAME" = "x" ] ;then
        (
        cd ${SD_KERNEL_OUT}
        arch_target="Image ${SD_KERN_DEVICETREE}"

        KERNEL_DEPOLY=${DEPLOYDIR}/${MACHINE_NAME}/
        mkdir -p ${KERNEL_DEPOLY}
        #The Image is origin binary from vmlinux.
        if [ -f arch/${ARCH}/boot/Image ]; then
            cp -vf arch/${ARCH}/boot/Image ${KERNEL_DEPOLY}/Image
        fi

        if [ -f arch/${ARCH}/boot/zImage ] || [ -f arch/${ARCH}/boot/uImage ]; then
            cp -vf arch/${ARCH}/boot/[zu]Image ${KERNEL_DEPOLY}/
        fi

        if [ -f arch/${ARCH}/boot/Image.gz ]; then
            cp -vf arch/${ARCH}/boot/Image.gz ${KERNEL_DEPOLY}/
        fi

        cp .config ${KERNEL_DEPOLY}/
        cp -vf arch/${ARCH}/boot/dts/${SD_KERN_DEVICETREE} ${KERNEL_DEPOLY}/
    #   tar -jcf ${KERNEL_DEPOLY}/vmlinux.tar.bz2 vmlinux
    
        SD_KERNEL_MOD_DEPOLY_DIR=${KERNEL_DEPOLY}/lib/modules/${KERNEL_VERSION}

        rm -rf  ${KERNEL_DEPOLY}/lib/modules/*
        mkdir -p ${SD_KERNEL_MOD_DEPOLY_DIR}
        for file in $(find drivers sound crypto block fs security net -name "*.ko"); do
             cp $file ${SD_KERNEL_MOD_DEPOLY_DIR}
        done
        cp -f Module.symvers ${SD_KERNEL_MOD_DEPOLY_DIR}
        )
    fi
    
)
}

function do_deploy_linux()
{
    cd ${SD_KERNEL_DIR}
    KERNEL_VERSION=`make -s kernelversion -C ./`
(  
    echo "deploy linux kernel "

    # Image is arm architecture specific target
    local arch_target=""

    if [ ! "x$MACHINE_NAME" = "x" ] ;then
        (
        cd ${SD_KERNEL_OUT}
        arch_target="Image ${SD_KERN_DEVICETREE}"

        KERNEL_DEPOLY=${DEPLOYDIR}/${MACHINE_NAME}/
        mkdir -p ${KERNEL_DEPOLY}
        #The Image is origin binary from vmlinux.
        if [ -f arch/${ARCH}/boot/Image ]; then
            cp -vf arch/${ARCH}/boot/Image ${KERNEL_DEPOLY}/Image
        fi

        if [ -f arch/${ARCH}/boot/zImage ] || [ -f arch/${ARCH}/boot/uImage ]; then
            cp -vf arch/${ARCH}/boot/[zu]Image ${KERNEL_DEPOLY}/
        fi

        if [ -f arch/${ARCH}/boot/Image.gz ]; then
            cp -vf arch/${ARCH}/boot/Image.gz ${KERNEL_DEPOLY}/
        fi

        cp .config ${KERNEL_DEPOLY}/
        cp -vf arch/${ARCH}/boot/dts/${SD_KERN_DEVICETREE} ${KERNEL_DEPOLY}/
    #   tar -jcf ${KERNEL_DEPOLY}/vmlinux.tar.bz2 vmlinux
    
        SD_KERNEL_MOD_DEPOLY_DIR=${KERNEL_DEPOLY}/lib/modules/${KERNEL_VERSION}

        rm -rf  ${KERNEL_DEPOLY}/lib/modules/*
        mkdir -p ${SD_KERNEL_MOD_DEPOLY_DIR}
        for file in $(find drivers sound crypto block fs security net -name "*.ko"); do
             cp $file ${SD_KERNEL_MOD_DEPOLY_DIR}
        done
        cp -f Module.symvers ${SD_KERNEL_MOD_DEPOLY_DIR}
        )
    fi
    
    if [ ! "x$MACHINE_NAME2" = "x" ] ;then
    (
        cd ${SD_KERNEL_OUT2}

        KERNEL_DEPOLY=${DEPLOYDIR}/${MACHINE_NAME2}/
        mkdir -p ${KERNEL_DEPOLY}
        
        #The Image is origin binary from vmlinux.
        if [ -f arch/${ARCH}/boot/Image ]; then
            cp -vf arch/${ARCH}/boot/Image ${KERNEL_DEPOLY}/Image
        fi

        if [ -f arch/${ARCH}/boot/zImage ] || [ -f arch/${ARCH}/boot/uImage ]; then
            cp -vf arch/${ARCH}/boot/[zu]Image ${KERNEL_DEPOLY}/
        fi

        if [ -f arch/${ARCH}/boot/Image.gz ]; then
            cp -vf arch/${ARCH}/boot/Image.gz ${KERNEL_DEPOLY}/
        fi

        cp .config ${KERNEL_DEPOLY}/
        cp -vf arch/${ARCH}/boot/dts/${SD_KERN_DEVICETREE2} ${KERNEL_DEPOLY}/
        SD_KERNEL_MOD_DEPOLY_DIR=${KERNEL_DEPOLY}/lib/modules/${KERNEL_VERSION}

        rm -rf  ${KERNEL_DEPOLY}/lib/modules/*
        echo "${SD_KERNEL_MOD_DEPOLY_DIR}"
        mkdir -p ${SD_KERNEL_MOD_DEPOLY_DIR}
        for file in $(find drivers sound crypto block fs security net -name "*.ko"); do
             cp $file ${SD_KERNEL_MOD_DEPOLY_DIR}
        done
        cp -f Module.symvers ${SD_KERNEL_MOD_DEPOLY_DIR}
    )
    fi
)
}

function do_compile_openwrt() {
    if [ "x$MACHINE_NAME" != "x" -a "x$MACHINE_ROOTFS_TYPE" = "xopenwrt" ] ;then
    (
		cd ${SD_TOPDIR}/source/openwrt
        if [ ! -e ${SD_TOPDIR}/source/openwrt/.config ];then
			./make_update.sh
		fi
        make -j${SD_KERN_JLEVEL}
    )
    fi
}

function do_deploy_openwrt() {
	if [ "x$MACHINE_NAME" != "x" -a "x$MACHINE_ROOTFS_TYPE" = "xopenwrt" ] ;then

		SD_OPENWRT_OUT_PATH=${SD_TOPDIR}/source/openwrt/build_dir/target-aarch64_generic_musl/linux-semidrive_armv8/

		if [ -h ${SD_KERNEL_OUT} ]; then rm $SD_KERNEL_OUT; fi
        ln -s ${SD_OPENWRT_OUT_PATH}/linux-5.10 ${SD_KERNEL_OUT}

        mkdir -p ${DEPLOYDIR}/${MACHINE_NAME}/

        if [ -e ${DEPLOYDIR}/${MACHINE_NAME}/rootfs.img ]; then rm ${DEPLOYDIR}/${MACHINE_NAME}/rootfs.img; fi

		if [ -e $SD_OPENWRT_OUT_PATH/root.ext4 ]; then
			ln -s $SD_OPENWRT_OUT_PATH/root.ext4  ${DEPLOYDIR}/${MACHINE_NAME}/rootfs.img
		fi

	fi

	do_deploy_linux5.10
}

function do_compile_rootfs_buildroot() {
    if [ "x$MACHINE_NAME" != "x" -a "x$MACHINE_ROOTFS_TYPE" = "xbuildroot" ] ;then
    (
        if [ ! -e ${SD_BR_OUT_PATH}/.config ];then
            cd ${SD_TOPDIR}/source/buildroot
            make ${MACHINE_BR_ROOTFS_CONFIG} O=${SD_BR_OUT_PATH}
        else
            cd ${SD_BR_OUT_PATH}
        fi
        make -j${SD_KERN_JLEVEL} O=${SD_BR_OUT_PATH} 
    )
    fi
    
    if [ "x$MACHINE_NAME2" != "x" -a "x$MACHINE_ROOTFS2_TYPE" = "xbuildroot" ] ;then
    (
        cd ${SD_TOPDIR}/source/buildroot
        make ${MACHINE_BR_ROOTFS2_CONFIG} O=${SD_BR_OUT_PATH2}
        make -j${SD_KERN_JLEVEL} O=${SD_BR_OUT_PATH2} 
    )
    fi
}

function do_deploy_rootfs_buildroot() {
    if [ "x$MACHINE_NAME" != "x" -a "x$MACHINE_ROOTFS_TYPE" = "xbuildroot" ] ;then
    (
        cd ${SD_BR_OUT_PATH}
        BR_DEPOLY_DIR=${DEPLOYDIR}/${MACHINE_NAME}/
        mkdir -p ${BR_DEPOLY_DIR}
        rm -rf ${BR_DEPOLY_DIR}/rootfs.img
        if [ -d images ]; then
            ln -s ${SD_BR_OUT_PATH}/images/rootfs.ext4  ${BR_DEPOLY_DIR}/rootfs.img
        fi
    )
    fi
    
    if [ "x$MACHINE_NAME2" != "x" -a "x$MACHINE_ROOTFS2_TYPE" = "xbuildroot" ] ;then
    (
        cd ${SD_BR_OUT_PATH2}
        BR_DEPOLY_DIR=${DEPLOYDIR}/${MACHINE_NAME2}/
        if [ ! -d ${BR_DEPOLY_DIR} ]; then mkdir -p ${BR_DEPOLY_DIR};fi
        rm  -rf ${BR_DEPOLY_DIR}/rootfs.img
        if [ -d ${SD_BR_OUT_PATH2}/images ]; then
            ln -s ${SD_BR_OUT_PATH2}/images/rootfs.cpio  ${BR_DEPOLY_DIR}/rootfs.img
        fi
    )
    fi
}

function do_compile_ubuntu()
{

    if [ "x$MACHINE_NAME" != "x" -a "x$MACHINE_ROOTFS_TYPE" = "xubuntu" ] ;then
    (
    cd ${SD_TOPDIR}/source/
    UBUNTU_DEPOLY_DIR=${DEPLOYDIR}/${MACHINE_NAME}/
    mkdir -p ${UBUNTU_DEPOLY_DIR}

    #install kernel modules
    KERNEL_DEPOLY=${DEPLOYDIR}/${MACHINE_NAME}/
    MOD_DEPLOY_DIR=${KERNEL_DEPOLY}/lib/modules
    rm ubuntu_rootfs/lib/modules -rf
    cp -r ${MOD_DEPLOY_DIR} ubuntu_rootfs/lib/
    
    ./ubuntu_rootfs/scripts/make_ext4fs -l ${MACHINE_ROOTFS_SIZE}M ${UBUNTU_DEPOLY_DIR}/rootfs.ext4 ubuntu_rootfs
    ./ubuntu_rootfs/scripts/sparse/img2simg ${UBUNTU_DEPOLY_DIR}/rootfs.ext4  ${UBUNTU_DEPOLY_DIR}/rootfs.img
    echo -e "UBUNTU_ROOTFS_EXT4 spare Build Package Name  : " $UBUNTU_DEPOLY_DIR/rootfs.img `stat -L -c %s ${UBUNTU_DEPOLY_DIR}/rootfs.img`
    )
    fi
    
    if [ "x$MACHINE_NAME2" != "x" -a "x$MACHINE_ROOTFS2_TYPE" = "xubuntu" ] ;then
    (
    cd ${SD_TOPDIR}/source/
    UBUNTU_DEPOLY_DIR=${DEPLOYDIR}/${MACHINE_NAME2}/
    mkdir -p ${UBUNTU_DEPOLY_DIR}

    #install kernel modules
    KERNEL_DEPOLY=${DEPLOYDIR}/${MACHINE_NAME2}/
    MOD_DEPLOY_DIR=${KERNEL_DEPOLY}/lib/modules
    rm ubuntu_rootfs/lib/modules -rf
    cp -r ${MOD_DEPLOY_DIR} ubuntu_rootfs/lib/
    
    ./ubuntu_rootfs/scripts/make_ext4fs -l ${MACHINE_ROOTFS2_SIZE}M ${UBUNTU_DEPOLY_DIR}/rootfs.ext4 ubuntu_rootfs
    ./ubuntu_rootfs/scripts/sparse/img2simg ${UBUNTU_DEPOLY_DIR}/rootfs.ext4  ${UBUNTU_DEPOLY_DIR}/rootfs.img
    echo -e "UBUNTU_ROOTFS_EXT4 spare Build Package Name  : " $UBUNTU_DEPOLY_DIR/rootfs.img `stat -L -c %s ${UBUNTU_DEPOLY_DIR}/rootfs.img`
    )
    fi
}

function do_ospi_uImage_pack() {
    if [ "x$MACHINE_NAME" != "x" -a "x$MACHINE_KERN_TYPE" = "xuImage" ] ;then
    (
        KERNEL_DEPOLY=${DEPLOYDIR}/${MACHINE_NAME}/
        if [[ $CHIPVERSION == *"plus"* ]]; then
            AP_STR="d9plus_ap1_ref";
        else
            AP_STR="${CHIPVERSION}_ref";
        fi
        #d9lite_ref  d9plus_ap1_ref  d9plus_ap2_ref  d9_ref
		if [ -e ${SD_TOPDIR}/source/u-boot/board/semidrive/${AP_STR}/${SD_UIMAGE_FDT_CONF} ]; then
			cp ${SD_TOPDIR}/source/u-boot/board/semidrive/${AP_STR}/${SD_UIMAGE_FDT_CONF} ${KERNEL_DEPOLY}
		fi
        #cp ${SD_TOPDIR}/source/u-boot/tools/mkimage  ${KERNEL_DEPOLY}
        #cd ${KERNEL_DEPOLY}
		if [ -e ${SD_UBOOT_OUT}/tools/mkimage ]; then
			${SD_UBOOT_OUT}/tools/mkimage -f ${KERNEL_DEPOLY}/${SD_UIMAGE_FDT_CONF} ${KERNEL_DEPOLY}/uImage
		fi
    )
    fi

    if [ "x$MACHINE_NAME2" != "x" -a "x$MACHINE_KERN_TYPE2" = "xuImage" ] ;then
    (
        if [[ $CHIPVERSION == *"plus"* ]]; then AP_STR="d9plus_ap2_ref";fi
        KERNEL_DEPOLY=${DEPLOYDIR}/${MACHINE_NAME2}/
        cp ${SD_TOPDIR}/source/u-boot/board/semidrive/${AP_STR}/${SD_UIMAGE_FDT_CONF2} ${KERNEL_DEPOLY}
        #cp ${SD_TOPDIR}/source/u-boot/tools/mkimage  ${KERNEL_DEPOLY}
        #cd ${KERNEL_DEPOLY}
        ${SD_UBOOT_OUT}/tools/mkimage -f ${KERNEL_DEPOLY}/${SD_UIMAGE_FDT_CONF2} ${KERNEL_DEPOLY}/uImage
    )
    fi
}

function do_pack() {
    if [ "x$SD_BOOT_MODE" == "xemmc_only" ];then
        boot_mode_flag="-e"
    elif [ "x$SD_BOOT_MODE" == "xsdcard" ];then
        boot_mode_flag="-c"
    else
        boot_mode_flag=""
    fi

    do_ospi_uImage_pack
    (
        cd ${SD_TOPDIR}/meta-semidrive/scripts/
        if [ "x${BAREMETAL_TGT}" != "x" ];then
            ./emmc_pac_separate.sh ${boot_mode_flag} -p ${MACHINE_PROJECT} -s ${CHIPVERSION}
        else
            ./emmc_pac_separate.sh -p ${MACHINE_PROJECT} -s ${CHIPVERSION}
            ./ospi_pac_separate.sh 
            #PROJECT_CFG_PATH=${SD_TOPDIR}/source/chipcfg/generate/${CHIPVERSION}/projects/${MACHINE_PROJECT}
            #${PROJECT_CFG_PATH}/pack_cfg/pack_ospi.sh
        fi
    )
}

function do_compile_uboot() {
    if [ "x$MACHINE_NAME" != "x" -a "x$MACHINE_BOOTLOADER_TYPE" = "xuboot" ] ;then
        (
            cd ${SD_TOPDIR}/source/u-boot
            make ARCH=arm distclean
            #./pre_build.sh ${CHIPVERSION} ${SD_BOOT_MODE}
            make ARCH=arm ${SD_UBOOT_DEFCONF} O=${SD_UBOOT_OUT}
            make ARCH=arm -j${SD_KERN_JLEVEL} CROSS_COMPILE=${CROSS_COMPILE} O=${SD_UBOOT_OUT}
            cp ${SD_UBOOT_OUT}/u-boot.bin ${DEPLOYDIR}/bootloader.bin
           # cp ${SD_UBOOT_OUT}/spl/u-boot-spl.bin ${DEPLOYDIR}/preloader.bin
        )
    fi
    
    if [ "x$MACHINE_NAME2" != "x" -a "x$MACHINE_BOOTLOADER2_TYPE" = "xuboot" ] ;then
        (
            cd ${SD_TOPDIR}/source/u-boot
            make ARCH=arm distclean
            #./pre_build.sh ${CHIPVERSION} ${SD_BOOT_MODE}
            make ARCH=arm ${SD_UBOOT_DEFCONF2} O=${SD_UBOOT_OUT2}
            make ARCH=arm -j${SD_KERN_JLEVEL} CROSS_COMPILE=${CROSS_COMPILE} O=${SD_UBOOT_OUT2}
            cp ${SD_UBOOT_OUT2}/u-boot.bin ${DEPLOYDIR}/bootloader2.bin
           # cp ${SD_UBOOT_OUT2}/spl/u-boot-spl.bin ${DEPLOYDIR}/preloader2.bin
        )
    fi
}
function do_deploy_uboot() {
    if [ "x$MACHINE_NAME" != "x" -a "x$MACHINE_BOOTLOADER_TYPE" = "xuboot" ] ;then
        (
            bbplain "install -m 0644 u-boot.bin ${DEPLOYDIR}/bootloader.bin"
            install -m 0644 ${SD_UBOOT_OUT}/u-boot.bin ${DEPLOYDIR}/bootloader.bin
          #  bbplain "install -m 0644 u-boot-spl.bin ${DEPLOYDIR}/preloader.bin"
         #   install -m 0644 ${SD_UBOOT_OUT}/spl/u-boot-spl.bin ${DEPLOYDIR}/preloader.bin
        )
    fi
    
    if [ "x$MACHINE_NAME2" != "x" -a "x$MACHINE_BOOTLOADER2_TYPE" = "xuboot" ] ;then
        (
            bbplain "install -m 0644 u-boot.bin ${DEPLOYDIR}/bootloader2.bin"
            install -m 0644 ${SD_UBOOT_OUT2}/u-boot.bin ${DEPLOYDIR}/bootloader2.bin
          #  bbplain "install -m 0644 u-boot-spl.bin ${DEPLOYDIR}/preloader2.bin"
          #  install -m 0644 ${SD_UBOOT_OUT}/spl/u-boot-spl.bin ${DEPLOYDIR}/preloader2.bin
        )
    fi
}

#ssdk
function do_compile_ssdk() {
    (
        cd ${SD_TOPDIR}/source/ssdk
        echo "make ${SD_SSDK_SAF} begin"

        make ${SD_SSDK_SAF}
	#cp -f out/build-${SD_SSDK_SAF}/ssdk.bin ${DEPLOYDIR}/safety.bin

        echo "make ${SD_SSDK_SAF} finish"
    )

    (
        cd ${SD_TOPDIR}/source/ssdk
        echo "make ${SD_SSDK_SEC}"
        make ${SD_SSDK_SEC}
        #cp -f out/build-${SD_SSDK_SEC}/ssdk.bin ${DEPLOYDIR}/ssystem.bin
        echo "make ${SD_SSDK_SEC} finish"
    )
}
function do_deploy_ssdk() {
    (
        SSDK_DIR=${SD_TOPDIR}/source/ssdk
	if [ -e  ${DEPLOYDIR}/safety.bin -o -h ${DEPLOYDIR}/safety.bin ];then
		rm ${DEPLOYDIR}/safety.bin
	fi
        ln -s ${SSDK_DIR}/out/build-${SD_SSDK_SAF}/ssdk.bin ${DEPLOYDIR}/safety.bin

	if [ -e ${DEPLOYDIR}/ssystem.bin -o -h ${DEPLOYDIR}/ssystem.bin ];then
		rm ${DEPLOYDIR}/ssystem.bin;
	fi
	ln -s ${SSDK_DIR}/out/build-${SD_SSDK_SEC}/ssdk.bin ${DEPLOYDIR}/ssystem.bin
    )
}

function do_pack_spiflashimg() {
    (
        if [ x${BAREMETAL_TGT} != x"" ];then
            echo "no support emmc"
        else
            (
                cd ${DEPLOYDIR}
                #temp 2M all 0xff
                tr '\000' '\377' < /dev/zero | dd of=spiflash.image.bin bs=1024 count=2k

                #pack sfs
                dd if=sfs.img of=spiflash.image.bin conv=notrunc
                #pack partition 17K @2 sector(eg 8K)
                dd if=ospi_bak_partition.img of=spiflash.image.bin bs=1k seek=8 count=17 conv=notrunc
                #dil @0x7000
                dd if=dil.bin.safe.signed of=spiflash.image.bin bs=1k seek=28 conv=notrunc
                #dilbak @0x27000
                dd if=dil.bin.safe.signed of=spiflash.image.bin bs=1k seek=156 conv=notrunc

                #ddr_init_seq.bin @0x47000
                dd if=ddr_init_seq.bin of=spiflash.image.bin bs=1 seek=290816 conv=notrunc
                #ddr_fw.bin @0x4f000
                dd if=ddr_fw.bin of=spiflash.image.bin bs=1 seek=323584 conv=notrunc

                #safety.bin @0x6f000
                dd if=safety.bin of=spiflash.image.bin bs=1 seek=454656 conv=notrunc

                #ssystem.bin.sec.signed @0xaf000
                dd if=ssystem.bin.sec.signed of=spiflash.image.bin bs=1 seek=716800 conv=notrunc

                #preloader.bin @0xef000
                dd if=preloader.bin of=spiflash.image.bin bs=1 seek=978944 conv=notrunc

                #sml.bin @0x10f000
                dd if=sml.bin of=spiflash.image.bin bs=1 seek=1110016 conv=notrunc

                #bootloader.bin @0x11f000
                dd if=bootloader.bin of=spiflash.image.bin bs=1 seek=1175552
                echo "spi nor image gen in ${DEPLOYDIR}/spiflash.image.bin"
            )
        fi
    )
}

case "$1" in
    bm)
        do_compile_bm
        do_deploy_bm
        ;;
    lk)
        do_compile_lk
        do_deploy_lk
    ;;
    uboot)
        do_compile_uboot
        do_deploy_uboot
    ;;
    ssdk)
        do_compile_ssdk
        do_deploy_ssdk
    ;;
    safety)
        do_compile_safety
        do_deploy_safety
        ;;
    linux |kernel)
        do_compile_linux
        do_deploy_linux
        ;;
    buildroot)
        do_compile_rootfs_buildroot
        do_deploy_rootfs_buildroot
        ;;
    ubuntu)
        do_compile_ubuntu
#       do_depoly_ubuntu
        ;;
	openwrt)
		do_compile_openwrt
		do_deploy_openwrt
		;;
    pack)
        do_pack
    ;;
    spiimg)
        do_pack_spiflashimg
    ;;
    uImage)
        do_ospi_uImage_pack
    ;;
    *)
    do_compile_bm
    do_deploy_bm
    do_compile_lk
    do_deploy_lk
    do_compile_safety
    do_deploy_safety
    do_compile_uboot
    do_deploy_uboot
    do_compile_ssdk
    do_deploy_ssdk
	do_compile_openwrt
	do_deploy_openwrt
#    do_compile_linux
#    do_deploy_linux
    #do_compile_rootfs_buildroot
    #do_deploy_rootfs_buildroot
    #do_compile_ubuntu
    do_pack
    echo -e "\n\033[0;31;1m compile Semidrive SDK and PACK successful\033[0m\n\n"
    ;;
esac
