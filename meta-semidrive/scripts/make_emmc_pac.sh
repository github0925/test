#!/bin/bash

echo $YOCTO_MACHINE $SECOND_MACHINE

usage()
{
    echo -e "\nUsage: make_emmc_pac.sh
    Optional parameters: [-y yocto-top] [-b yocto-build-name] [-h] [-m yocto machine]"
    echo "
    * [-y yocto-top-path]:   Yocto top directory, default:$YOCTO_TOP
    * [-b yocto-build]:      Main build dir, default env:$YOCTO_BUILD_DIR
    * [-d 2rd-build]:        Second build absolut path, default:$SECOND_BUILD_DIR
    * [-m yocto-machine]:    Yocto machine [x9h_evb|gx9_evb|x9h_ref], default:$YOCTO_MACHINE
    * [-s signed-spl]:       Signed spl file, default:`pwd`/$SIGNED_SPL
    * [-p package]:          Global Package [x9hplus_evb], default:$PACKAGE
    * [-e]:                  All images in eMMC,use the bpt file with suffix '_emmc_only'
    * [-h]:                  This help message
"
}

clean_up()
{
    unset make_pac_help make_pac_error make_pac_flag
    unset usage clean_up
}

# Now, there is a dtbo partition in every global.bpt,
# but there is no dtbo image in some machine, so using a fake dtbo.
# If there is a real dtbo image in some machine in the future,
# it needs to remove the call for creating fake dtbo image and add the real dtbo image
# to the ${VBMTEA_DESCIPTOR} and ${SPECIFIED_PARTITION_PACK}
function make_fake_dtbo_for_vbmeta()
{
    DTBO_PATH=${YOCTO_IMAGE_DIR}/dtbo_fake.img
    dd if=/dev/zero of=${DTBO_PATH} bs=1024 count=4
    VBMTEA_DESCIPTOR+=" ${DTBO_PATH}:dtbo:${HASH_ALG}:hash"
    SPECIFIED_PARTITION_PACK+=" --image dtbo:${DTBO_PATH} "
}

function convert_sparse_image()
{
    local img_list=$1
    for raw_img in ${img_list[*]};do
        raw_img=`realpath $raw_img`
        file_type=$(file $raw_img)
        if [[ "${file_type}" =~ "sparse image" ]];then
            echo "$raw_img is already sparse image!"
            continue
        fi
        ./img2simg ${raw_img} ${raw_img}.sparse
        mv ${raw_img}.sparse ${raw_img}
    done
}

source ./sign_tool/sign_helper.sh
export PATH=.:${PWD}/sign_tool/:${PWD}/sign_tool/fec:${PATH}
export LD_LIBRARY_PATH=.:${PWD}/sign_tool/fec:${LD_LIBRARY_PATH}

# get command line options
OLD_OPTIND=$OPTIND

YOCTO_TOP=`realpath ../..`
YOCTO_BUILD_DIR=${BUILDDIR}
KERNEL_PREFIX=x9_evb
PRODUCT=x9
YOCTO_ROOTFS_IMAGE=core-image-base
SECOND_MACHINE=""
SECOND_KERNEL_DTS=""
SECOND_BUILD_DIR=""

GLOBAL_BPT_SUFFIX=
UNIFIED_BOOT_EXTRA=
while getopts "y:s:m:b:d:p:ech" make_pac_flag
do
    case $make_pac_flag in
        y) YOCTO_TOP="$OPTARG";
           ;;
        b) YOCTO_BUILD_DIR="${YOCTO_TOP}/$OPTARG";
           ;;
        d) SECOND_BUILD_DIR="$OPTARG";
           ;;
        m) YOCTO_MACHINE="$OPTARG";
           PACKAGE=${YOCTO_MACHINE}
           ;;
        s) SIGNED_SPL="$OPTARG";
           ;;
        p) PACKAGE="$OPTARG";
           ;;
        e) GLOBAL_BPT_SUFFIX="_emmc_only";
           ;;
        c) GLOBAL_BPT_SUFFIX="_sdcard_only";
           ;;
        h) make_pac_help='true';
           ;;
        \?) make_pac_error='true';
           ;;
    esac
done

shift $((OPTIND-1))
if [ $# -ne 0 ]; then
    echo -e "Invalid command line ending: '$@'"
fi
OPTIND=$OLD_OPTIND
if test $make_pac_help; then
    usage && clean_up && exit 1
elif test $make_pac_error; then
    clean_up && exit 1
fi

# Package may contain one or more machines, if not defined, try the default machine
if [ "x$PACKAGE" == "x" ]; then
    PACKAGE=$YOCTO_MACHINE
fi

if [ "xd9_ref" == "x$PACKAGE" ]; then
    KERNEL_PREFIX=d9_std_d9340_ref
    PRODUCT=d9
fi

if [ "xd9lite_ref" == "x$PACKAGE" ]; then
    KERNEL_PREFIX=d9_lite_d9310_ref
    PRODUCT=d9
fi

AP_STR==`echo $PACKAGE | sed "s/d9plus_ref//"`
echo "AP_STR=$AP_STR"
#x9p x9plus x9plus_ap1 x9plus_ref_ap1 
if [[ "x$PACKAGE" == "xd9plus_ref"* ]]; then
    KERNEL_PREFIX=d9_plus_d9350_ap1_ref
    PRODUCT=d9
    PACKAGE="d9plus_ref"
    YOCTO_MACHINE=d9plus_ref_ap1
    SECOND_MACHINE=d9plus_ref_ap2
    SECOND_KERNEL_DTS=d9_plus_d9350_ap2_ref.dtb

    SECOND_BUILD_DIR=${YOCTO_TOP}/build-d9plus_ap2
    SECOND_IMAGE_DIR=${SECOND_BUILD_DIR}/tmp/deploy/images/${SECOND_MACHINE}
#    SECOND_IMAGE=${SECOND_IMAGE_DIR}/Image
    SECOND_DTB=${SECOND_IMAGE_DIR}/${SECOND_KERNEL_DTS}
    SECOND_ROOTFS=${SECOND_IMAGE_DIR}/rootfs.img
fi

if [ -h $YOCTO_MACHINE ];then
    echo "$YOCTO_MACHINE soft link exist"
    rm $YOCTO_MACHINE
fi

CHIPVERSION=`echo $YOCTO_MACHINE | sed "s/_ref//" | sed "s/_ap.//"`
if [ -h $YOCTO_MACHINE ];then rm $YOCTO_MACHINE;fi
ln -s ../../source/lk_customize/chipcfg/generate/$CHIPVERSION/projects/$PACKAGE/pack_cfg $YOCTO_MACHINE

## check the necessary files
YOCTO_IMAGE_DIR=${YOCTO_BUILD_DIR}/tmp/deploy/images/${YOCTO_MACHINE}
if [ ! -e $YOCTO_IMAGE_DIR ]; then
    echo -e "\n ERROR - No yocto Image found in the path" $YOCTO_IMAGE_DIR
    echo -e "\n use 'bitbake virtual/kernel -C compile' command to build"
    exit 1
fi

ATB_SIGN_KEY=./sign_tool/vbmeta/keys/root-key.pem
SIGNED_SPL=${PACKAGE}/prebuilts/spl.bin.signed.rsa1024
SIGNED_HANDOVER=${PACKAGE}/prebuilts/ospihandover.bin.safe.signed
#ATF_FILE=${PACKAGE}/prebuilts/sml.bin

ATF_FILE=${YOCTO_IMAGE_DIR}/sml.bin
if [ ! -e ${ATF_FILE} ];then ATF_FILE=$YOCTO_MACHINE/prebuilts/sml.bin;fi
SECOND_ATF_FILE=${SECOND_IMAGE_DIR}/sml2.bin
if [ ! -e ${SECOND_ATF_FILE} ];then SECOND_ATF_FILE=$YOCTO_MACHINE/prebuilts/sml2.bin;fi

if [ ! -e ${YOCTO_IMAGE_DIR}/ssystem.bin ];then
    echo "cp -f $YOCTO_MACHINE/prebuilts/ssystem.bin $YOCTO_IMAGE_DIR/ssystem.bin"
    cp -f $YOCTO_MACHINE/prebuilts/ssystem.bin $YOCTO_IMAGE_DIR/ssystem.bin;
fi

echo -e " TOP directory  : " $YOCTO_TOP
echo -e " Build directory: " $YOCTO_BUILD_DIR $SECOND_BUILD_DIR
echo -e " Build Package  : " $PACKAGE
echo -e " Build machine  : " $YOCTO_MACHINE $SECOND_MACHINE
echo -e " Linux dts      : " $KERNEL_PREFIX $SECOND_KERNEL_DTS
echo -e " Signed spl path: "`pwd`/$SIGNED_SPL
echo -e " ATF binary path: "`pwd`/$ATF_FILE

if [ ! -e $YOCTO_IMAGE_DIR/ssystem.bin ]; then
    echo -e "\n ERROR - No ssystem found in the path" $YOCTO_IMAGE_DIR
    echo -e "\n try 'bitbake lk' command to build"
    exit 1
fi
if [ ! -e $YOCTO_IMAGE_DIR/preloader.bin ]; then
    echo -e "\n ERROR - No preloader found in the path" $YOCTO_IMAGE_DIR
    echo -e "\n try 'bitbake lk' command to build"
    exit 1
fi

## when use ext4 real rootfs, the ramdisk is not used. for compatible only, set a fake file
YOCTO_ROOTFS=${YOCTO_IMAGE_DIR}/${YOCTO_ROOTFS_IMAGE}-${YOCTO_MACHINE}.manifest
## YOCTO_ROOTFS=${YOCTO_IMAGE_DIR}/${YOCTO_ROOTFS_IMAGE}-${YOCTO_MACHINE}.cpio.gz
YOCTO_ROOTFS_EXT4=${YOCTO_IMAGE_DIR}/${YOCTO_ROOTFS_IMAGE}-${YOCTO_MACHINE}.ext4

if [ ! -e $YOCTO_ROOTFS_EXT4 ]; then
    echo -e "\n No ${YOCTO_ROOTFS_IMAGE} rootfs not found in the path" $YOCTO_ROOTFS_EXT4
    echo -e "Try the minimal rootfs"
    YOCTO_ROOTFS_EXT4=`find ${YOCTO_IMAGE_DIR} -name *${YOCTO_MACHINE}.ext4`
    YOCTO_ROOTFS=`find ${YOCTO_IMAGE_DIR} -name *${YOCTO_MACHINE}.manifest`
    if [ ! -e $YOCTO_ROOTFS_EXT4 ]; then
        echo -e "\n ERROR - No rootfs not found in the path" $YOCTO_ROOTFS_EXT4
        echo -e "\n Please use bitbake core-image-base or bitbake core-image-minimal to make a rootfs"
        echo -e "\n"
        exit 1
    fi
fi


MKIMAGE_PATH="source/u-boot/tools/mkimage"
if [ ${YOCTO_MACHINE} == "d9lite_ref" ]; then
    MACHINE_TEMP=${YOCTO_MACHINE}
    SD_UIMAGE_FDT_CONF_PATH="source/u-boot/board/semidrive/${MACHINE_TEMP}"
    SD_UIMAGE_FDT_CONF="d9lite_fdt.its"
elif [ ${YOCTO_MACHINE} == "d9_ref" ]; then
    MACHINE_TEMP=${YOCTO_MACHINE}
    SD_UIMAGE_FDT_CONF_PATH="source/u-boot/board/semidrive/${MACHINE_TEMP}"
    SD_UIMAGE_FDT_CONF="d9_fdt.its"
elif [ ${YOCTO_MACHINE} == "d9plus_ref_ap1" ]; then
    MACHINE_TEMP="d9plus_ap1_ref"
    SD_UIMAGE_FDT_CONF_PATH="source/u-boot/board/semidrive/${MACHINE_TEMP}"
    SD_UIMAGE_FDT_CONF="d9plus_ap1_fdt.its"
    
    SECOND_MACHINE_TEMP="d9plus_ap2_ref"
    SECOND_SD_UIMAGE_FDT_CONF_PATH="source/u-boot/board/semidrive/${SECOND_MACHINE_TEMP}"
    SECOND_SD_UIMAGE_FDT_CONF="d9plus_ap2_fdt.its"
    SECOND_MKIMAGE=true
fi


if [ ${YOCTO_BL_TYPE} == "uboot" ]; then
    (
        cp ${YOCTO_TOP}/${SD_UIMAGE_FDT_CONF_PATH}/${SD_UIMAGE_FDT_CONF} ${YOCTO_IMAGE_DIR}
        cp ${YOCTO_TOP}/${MKIMAGE_PATH}  ${YOCTO_IMAGE_DIR}
        cd ${YOCTO_IMAGE_DIR}
        ./mkimage -f ${YOCTO_IMAGE_DIR}/${SD_UIMAGE_FDT_CONF} ${YOCTO_IMAGE_DIR}/uImage
    )
    YOCTO_IMAGE=${YOCTO_IMAGE_DIR}/uImage
    if [ ${SECOND_MKIMAGE} == true ]; then
        (
	    cp ${SECOND_IMAGE_DIR}/*.rootfs.cpio ${SECOND_IMAGE_DIR}/rootfs.img
            cp ${YOCTO_TOP}/${SECOND_SD_UIMAGE_FDT_CONF_PATH}/${SECOND_SD_UIMAGE_FDT_CONF} ${SECOND_IMAGE_DIR}
            cp ${YOCTO_TOP}/${MKIMAGE_PATH}  ${SECOND_IMAGE_DIR}
            cd ${SECOND_IMAGE_DIR}
            ./mkimage -f ${SECOND_IMAGE_DIR}/${SECOND_SD_UIMAGE_FDT_CONF} ${SECOND_IMAGE_DIR}/uImage
        )
       SECOND_IMAGE=${SECOND_IMAGE_DIR}/uImage 
    fi
else
    YOCTO_IMAGE=${YOCTO_IMAGE_DIR}/Image
fi

#YOCTO_IMAGE=${YOCTO_IMAGE_DIR}/Image
if [ ! -e $YOCTO_IMAGE ]; then
    echo -e "\n ERROR - No kernel image found in the path" $YOCTO_IMAGE
    echo -e "\n"
    exit 1
fi

YOCTO_DTB=${YOCTO_IMAGE_DIR}/${KERNEL_PREFIX}.dtb
if [ ! -e $YOCTO_DTB ]; then
    echo -e "\n ${KERNEL_PREFIX}.dtb not found in the path" ${YOCTO_IMAGE_DIR}
    YOCTO_DTB=`find ${YOCTO_IMAGE_DIR} -name ${PRODUCT}_*.dtb | head -n 1`
    echo -e "Try variants: " $YOCTO_DTB
    if [ ! -e $YOCTO_DTB ]; then
        echo -e "\n ERROR - No dtb found in the path" $YOCTO_DTB
        exit 1
    fi
fi

if [ -e sign_tool/run_sign_sec ]; then
    ./sign_tool/run_sign_sec -i ${YOCTO_IMAGE_DIR}/spl.bin -k ${ATB_SIGN_KEY}
    SIGNED_SPL=${YOCTO_IMAGE_DIR}/spl.bin.sec.signed
    SIGNED_HANDOVER=${YOCTO_IMAGE_DIR}/ospihandover.bin.signed
else
    SIGNED_SPL=${YOCTO_MACHINE}/prebuilts/spl.bin.signed.rsa1024
    SIGNED_HANDOVER=${YOCTO_MACHINE}/prebuilts/ospihandover.bin.safe.signed
fi
echo -e "Use Signed SPL image" $SIGNED_SPL

DA_COMMON_ARGS=" --product ${PRODUCT} --da FDA:$SIGNED_SPL --da OPSIDA:$SIGNED_HANDOVER --da DLOADER:${YOCTO_IMAGE_DIR}/dloader.bin "
SPECIFIED_PARTITION_PACK+=" --allow_empty_partitions "

IMG2SIMG_LIST=
BPT_ORIG_FILE=
BPT_OUT_FILE=
BPT_OUT_IMAGE=
HASH_ALG=sha256
KEY_FILE="./sign_tool/vbmeta/keys/vbmeta-user-key.pem"
KEY_META="./sign_tool/vbmeta/keys/vbmeta-user-rsa2048whitsha256-romtest.cer.chain"
VBMETA_SIGNED_ALG=SHA256_RSA2048
VBMETA_SIGNED_ARGS="--algorithm ${VBMETA_SIGNED_ALG} --key ${KEY_FILE} --public_key_meta ${KEY_META}"
VBMTEA_DESCIPTOR=

#atf file will be add footer, so copy it to temp dir
#cp -f ${PACKAGE}/prebuilts/sml.bin  ${YOCTO_IMAGE_DIR}/sml.bin
#ATF_FILE=${YOCTO_IMAGE_DIR}/sml.bin
#if [ ! -e ${ATF_FILE} ];then ATF_FILE=$YOCTO_MACHINE/prebuilts/sml.bin;fi


rm out/*.bpt -rf
mkdir out -p
GLOBAL_OUT=$YOCTO_BUILD_DIR/global.pac

if [ "xd9_ref" == "x$PACKAGE" ]; then
    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/d9/projects/default/system_config.bin
    BPT_ORIG_FILE=$YOCTO_MACHINE/global$GLOBAL_BPT_SUFFIX.bpt

    if [ ! -e $SYSTEM_CONFIG_BIN ];then
        SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/${CHIPVERSION}/projects/$PACKAGE/system_config.bin
    fi

    BPT_OUT_FILE=out/GPT_global_output.bpt
    BPT_OUT_IMAGE=out/GPT_global_partition.img

    echo "making a ext4 spare data partition"
    dd if=/dev/zero of=out/data.ext4 bs=1024 count=131072
    mkfs.ext4 out/data.ext4

    make_fake_dtbo_for_vbmeta

    SPECIFIED_PARTITION_PACK+=" --image kernel:${YOCTO_IMAGE}
                                --image dtb:${YOCTO_DTB}
                                --image rootfs:${YOCTO_ROOTFS_EXT4}
                                --image userdata:out/data.ext4
                                --image atf:${ATF_FILE}
                                --image bootloader:${YOCTO_IMAGE_DIR}/ivi_bootloader.bin
                                --version 2021W02 "

    VBMTEA_DESCIPTOR+=" ${YOCTO_DTB}:dtb:${HASH_ALG}:hash
                        ${YOCTO_IMAGE}:kernel:${HASH_ALG}:hash
                        ${ATF_FILE}:atf:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/ivi_bootloader.bin:bootloader:${HASH_ALG}:hash
                        ${YOCTO_ROOTFS_EXT4}:rootfs:${HASH_ALG}:hashtree"

    IMG2SIMG_LIST+=" ${YOCTO_ROOTFS_EXT4}"
fi


if [ "xd9plus_ref" == "x$PACKAGE" ]; then
    echo -e "\n>>> Generating bpt files"
    #BPT_ORIG_FILE=$PACKAGE/global$GLOBAL_BPT_SUFFIX.bpt
    BPT_ORIG_FILE=$YOCTO_MACHINE/global$GLOBAL_BPT_SUFFIX.bpt
    BPT_OUT_FILE=out/GPT_global_output.bpt
    BPT_OUT_IMAGE=out/GPT_global_partition.img
    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/d9plus/projects/default/system_config.bin
    if [ ! -e $SYSTEM_CONFIG_BIN ];then
        SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/${CHIPVERSION}/projects/$PACKAGE/system_config.bin
    fi

    echo -e "\n>>> Making a ext4 spare data partition"
    dd if=/dev/zero of=out/data.ext4 bs=1024 count=131072
    mkfs.ext4 out/data.ext4

    if [ ! -e $SECOND_DTB ]; then
        echo -e "\n ${SECOND_DTB} not found in the path" ${SECOND_IMAGE_DIR}
        SECOND_DTB=`find ${SECOND_IMAGE_DIR} -name ${PRODUCT}_*.dtb`
        echo -e "Try variants: " $SECOND_DTB
        if [ ! -e $SECOND_DTB ]; then
            echo -e "\n ERROR - No dtb found in the path" $SECOND_DTB
            exit 1
        fi
    fi

    echo -e "\n>>> Making global pac" ${GLOBAL_OUT}

    make_fake_dtbo_for_vbmeta

    SPECIFIED_PARTITION_PACK+=" --image dtb:${YOCTO_DTB}
                                --image kernel:${YOCTO_IMAGE}
                                --image rootfs:${YOCTO_ROOTFS_EXT4}
                                --image userdata:out/data.ext4
                                --image cluster_dtb:${SECOND_DTB}
                                --image cluster_preloader:${SECOND_IMAGE_DIR}/preloader.bin
                                --image cluster_bootloader:${SECOND_IMAGE_DIR}/ivi_bootloader.bin
                                --image cluster_kernel:${SECOND_IMAGE}
                                --image cluster_ramdisk:${SECOND_ROOTFS}
                                --image atf:${ATF_FILE}
                                --image cluster_atf:${SECOND_ATF_FILE}
                                --image bootloader:${YOCTO_IMAGE_DIR}/ivi_bootloader.bin
                                --version 2020W02 "

    VBMTEA_DESCIPTOR+=" ${YOCTO_DTB}:dtb:${HASH_ALG}:hash
                        ${YOCTO_IMAGE}:kernel:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/ivi_bootloader.bin:bootloader:${HASH_ALG}:hash
                        ${YOCTO_ROOTFS_EXT4}:rootfs:${HASH_ALG}:hashtree"
    IMG2SIMG_LIST+=" ${YOCTO_ROOTFS_EXT4}"
fi

if [ "xd9lite_ref" == "x$PACKAGE" ]; then
    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/d9lite/projects/default/system_config.bin
    BPT_ORIG_FILE=$YOCTO_MACHINE/global$GLOBAL_BPT_SUFFIX.bpt
    BPT_OUT_FILE=out/GPT_global_output.bpt
    BPT_OUT_IMAGE=out/GPT_global_partition.img
    if [ ! -e $SYSTEM_CONFIG_BIN ];then
        SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/${CHIPVERSION}/projects/$PACKAGE/system_config.bin
    fi

    echo "making a ext4 spare data partition"
    dd if=/dev/zero of=out/data.ext4 bs=1024 count=131072
    mkfs.ext4 out/data.ext4

    make_fake_dtbo_for_vbmeta

    SPECIFIED_PARTITION_PACK+=" --image kernel:${YOCTO_IMAGE}
                                --image dtb:${YOCTO_DTB}
                                --image rootfs:${YOCTO_ROOTFS_EXT4}
                                --image userdata:out/data.ext4
                                --image atf:${ATF_FILE}
                                --image bootloader:${YOCTO_IMAGE_DIR}/ivi_bootloader.bin
                                --version 2021W02 "

    VBMTEA_DESCIPTOR+=" ${YOCTO_DTB}:dtb:${HASH_ALG}:hash
                        ${YOCTO_IMAGE}:kernel:${HASH_ALG}:hash
                        ${ATF_FILE}:atf:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/ivi_bootloader.bin:bootloader:${HASH_ALG}:hash
                        ${YOCTO_ROOTFS_EXT4}:rootfs:${HASH_ALG}:hashtree"

    IMG2SIMG_LIST+=" ${YOCTO_ROOTFS_EXT4}"
fi

#sign dloader
image_add_hash_footer_no_bpt "${YOCTO_IMAGE_DIR}/dloader.bin" "dloader" "262144" "${HASH_ALG}" "${VBMETA_SIGNED_ARGS}"

if [ "x$GLOBAL_BPT_SUFFIX" != "x" ];then

    RES_IMAGE_SIZE=$(get_image_max_size "${BPT_ORIG_FILE}" "res" "hash" )
    RES_IMAGE_SIZE=$((RES_IMAGE_SIZE/1048576))

    mkdir -p ${YOCTO_IMAGE_DIR}/res_img/early_app
    cp -r ../../source/lk_safety/res/early_app/BootAnimation ${YOCTO_IMAGE_DIR}/res_img/early_app/
    cp -r ../../source/lk_safety/res/early_app/audio ${YOCTO_IMAGE_DIR}/res_img/early_app/

    ./dir2fat.sh -f -F 16 -S 512 ${YOCTO_IMAGE_DIR}/fat_res.img ${RES_IMAGE_SIZE} ${YOCTO_IMAGE_DIR}/res_img;

    ./sign_tool/run_sign_sec -i ${YOCTO_IMAGE_DIR}/dil.bin -l 0x1C0000 -e 0x1C0000 -k ${ATB_SIGN_KEY}
    ./sign_tool/run_sign_sec -i ${YOCTO_IMAGE_DIR}/ssystem.bin -l 0x140000 -e 0x140000 -k ${ATB_SIGN_KEY}
    ./sign_tool/run_sign_safe -i ${YOCTO_IMAGE_DIR}/dil2-unsigned.bin -l 0x100000 -e 0x100000 -k ${ATB_SIGN_KEY}
    mv ${YOCTO_IMAGE_DIR}/dil2-unsigned.bin.safe.signed ${YOCTO_IMAGE_DIR}/dil2.bin

    UNIFIED_BOOT_EXTRA+=" --preload spl:${YOCTO_IMAGE_DIR}/dil.bin.sec.signed
                          --image dil2:${YOCTO_IMAGE_DIR}/dil2.bin
                          --image safety_os:${YOCTO_IMAGE_DIR}/safety.bin
                          --image preloader:${YOCTO_IMAGE_DIR}/preloader.bin
                          --image ddr_fw:${YOCTO_IMAGE_DIR}/ddr_fw.bin
                          --image ddr_init_seq:${YOCTO_IMAGE_DIR}/ddr_init_seq.bin
                          --image ssystem:${YOCTO_IMAGE_DIR}/ssystem.bin.sec.signed
                          --image res:${YOCTO_IMAGE_DIR}/fat_res.img "

    VBMTEA_DESCIPTOR+=" ${YOCTO_IMAGE_DIR}/dil2.bin:dil2:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/ddr_init_seq.bin:ddr_init_seq:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/ddr_fw.bin:ddr_fw:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/preloader.bin:preloader:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/safety.bin:safety_os:${HASH_ALG}:hash"
    # for ssystem warm boot, use avb2.0 to verify ssystem image, so add its info to vbmeta
    VBMTEA_DESCIPTOR+=" ${YOCTO_IMAGE_DIR}/ssystem.bin.sec.signed:ssystem:${HASH_ALG}:hash"
    VBMTEA_DESCIPTOR+=" ${YOCTO_IMAGE_DIR}/fat_res.img:res:${HASH_ALG}:hash"

    SPECIFIED_PARTITION_PACK+=" --image system_config:${SYSTEM_CONFIG_BIN} "
    VBMTEA_DESCIPTOR+=" ${SYSTEM_CONFIG_BIN}:system_config:${HASH_ALG}:hash"

    if [ "xx9m_ref_serdes" == "x$PACKAGE" -o "xx9m_refa04_serdes" == "x$PACKAGE" -o "xx9m_ref" == "x$PACKAGE" ]; then
        fda_spl_bin=${YOCTO_IMAGE_DIR}/fda_spl.bin
        cp ${SIGNED_SPL} ${fda_spl_bin}
        UNIFIED_BOOT_EXTRA+=" --image fda_spl:${fda_spl_bin} "
        VBMTEA_DESCIPTOR+=" ${fda_spl_bin}:fda_spl:${HASH_ALG}:hash"
    fi
else
    UNIFIED_BOOT_EXTRA+=" --preload spl:$SIGNED_SPL "
fi

echo "BPT_ORIG_FILE:"${BPT_ORIG_FILE}

VBMETA_IMAGE_EMMC=${YOCTO_IMAGE_DIR}/vbmeta-emmc.img

echo "-----------VBMTEA_DESCIPTOR before :"${VBMTEA_DESCIPTOR}
VBMTEA_DESCIPTOR=$(images_add_footer ${BPT_ORIG_FILE} "${VBMTEA_DESCIPTOR}")
echo "VBMTEA_DESCIPTOR:"${VBMTEA_DESCIPTOR}


make_vbmeta_image "${VBMTEA_DESCIPTOR}" "${VBMETA_SIGNED_ARGS}" "${VBMETA_IMAGE_EMMC}" || exit 1
SPECIFIED_PARTITION_PACK+=" --image vbmeta:${VBMETA_IMAGE_EMMC} "

echo "IMG2SIMG_LIST:"${IMG2SIMG_LIST}
convert_sparse_image "${IMG2SIMG_LIST}"
python ./bpttool make_table --input ${BPT_ORIG_FILE} --ab_suffixes "_a,_b" --output_json ${BPT_OUT_FILE} --output_gpt ${BPT_OUT_IMAGE}
test $? -eq 0 || exit 1

python ./pactool make_pac_image --output $GLOBAL_OUT --input ${BPT_OUT_FILE} \
        ${DA_COMMON_ARGS} \
        ${UNIFIED_BOOT_EXTRA} \
        ${SPECIFIED_PARTITION_PACK}
test $? -eq 0 || exit 1

./gen_pack_crc ${GLOBAL_OUT}

echo -e "\n File $GLOBAL_OUT is generated Size:" `stat -L -c %s $GLOBAL_OUT`
