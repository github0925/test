#!/bin/bash
set -x
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
    * [-p package]:          Global Package [x9hplus_evb], default:${PACKAGE}
    * [-e]:                  All images in eMMC,use the bpt file with suffix '_emmc_only'
    * [-h]:                  This help message
"
}

clean_up()
{
    unset make_pac_help make_pac_error make_pac_flag
    unset usage clean_up
}
function print_warn()
{
	echo -e "\033[47;31m $*\033[0m"
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

SDK_TOP=`realpath ../..`
YOCTO_BUILD_DIR=${BUILDDIR}
YOCTO_ROOTFS_IMAGE=core-image-base
SECOND_MACHINE=""
SECOND_KERNEL_DTS=""
SECOND_BUILD_DIR=""

GLOBAL_BPT_SUFFIX=
UNIFIED_BOOT_EXTRA=
CHIP_VERSION=d9
MACHINE_PROJECT=d9_ref

# get command line options
OLD_OPTIND=$OPTIND
while getopts "y:s:m:b:d:p:ech" make_pac_flag
do
    case $make_pac_flag in
        y) SDK_TOP="$OPTARG";
           ;;
        b) YOCTO_BUILD_DIR="${SDK_TOP}/$OPTARG";
           ;;
        d) SECOND_BUILD_DIR="$OPTARG";
           ;;
        m) YOCTO_MACHINE="$OPTARG";
           PACKAGE=${YOCTO_MACHINE}
           ;;
        s) CHIP_VERSION="$OPTARG";
           ;;
        p) MACHINE_PROJECT="$OPTARG";
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


#input section
#BPT_ORIG_FILE
#UNIFIED_BOOT_EXTRA
#SPECIFIED_PARTITION_PACK
#SYSTEM_CONFIG_BIN
#ATF_FILE=
PROJECT_CFG_PATH=${SDK_TOP}/source/chipcfg/generate/${CHIP_VERSION}/projects/${MACHINE_PROJECT}
print_warn "---------------------CALLING PACK_CFG ${PROJECT_CFG_PATH}/pack_cfg/pack_cfg.sh -------------------------------"
source ${PROJECT_CFG_PATH}/pack_cfg/pack_cfg.sh

#output section


BPT_OUT_FILE=out/GPT_global_output.bpt
BPT_OUT_IMAGE=out/GPT_global_partition.img
GLOBAL_OUT=`cd ${DEPLOYDIR}/../images;pwd -P`
GLOBAL_OUT=${GLOBAL_OUT}/global.pac

rm out/*.bpt -rf
mkdir out -p

ATB_SIGN_KEY=./sign_tool/vbmeta/keys/root-key.pem
if [ -e sign_tool/run_sign_sec ]; then
    ./sign_tool/run_sign_sec -i ${DEPLOYDIR}/spl.bin -k ${ATB_SIGN_KEY}
    SIGNED_SPL=${DEPLOYDIR}/spl.bin.sec.signed
    SIGNED_HANDOVER=${DEPLOYDIR}/ospihandover.bin.signed
else
    SIGNED_SPL=${PROJECT_CFG_PATH}/pack_cfg/prebuilts/spl.bin.signed.rsa1024
    SIGNED_HANDOVER=${PROJECT_CFG_PATH}/pack_cfg/prebuilts/ospihandover.bin.safe.signed
fi
print_warn "Use Signed SPL image" $SIGNED_SPL " \n"


if [ "x$GLOBAL_BPT_SUFFIX" != "x" ];then
	print_warn "---------------------------------------SIGNED SOME BIN----------------${GLOBAL_BPT_SUFFIX}------------------------"
    RES_IMAGE_SIZE=$(get_image_max_size "${BPT_ORIG_FILE}" "res" "hash" )
    RES_IMAGE_SIZE=$((RES_IMAGE_SIZE/1048576))

    mkdir -p ${DEPLOYDIR}/res_img/early_app
    cp -r ../../source/lk_safety/res/early_app/BootAnimation ${DEPLOYDIR}/res_img/early_app/
    ./dir2fat.sh -f -F 16 -S 512 ${DEPLOYDIR}/fat_res.img ${RES_IMAGE_SIZE} ${DEPLOYDIR}/res_img;

    ./sign_tool/run_sign_sec -i ${DEPLOYDIR}/dil.bin -l 0x1C0000 -e 0x1C0000 -k ${ATB_SIGN_KEY}
    ./sign_tool/run_sign_sec -i ${DEPLOYDIR}/ssystem.bin -l 0x140000 -e 0x140000 -k ${ATB_SIGN_KEY}
    ./sign_tool/run_sign_safe -i ${DEPLOYDIR}/dil2-unsigned.bin -l 0x100000 -e 0x100000 -k ${ATB_SIGN_KEY}
    mv ${DEPLOYDIR}/dil2-unsigned.bin.safe.signed ${DEPLOYDIR}/dil2.bin

    is_ssdk=$(echo `readlink -f ${DEPLOYDIR}/ssystem.bin` | grep "ssdk.bin")
    if [[ "x$is_ssdk" != "x" ]];then
        echo "ssytem is powerd by ssdk"
        SSYSTEM_BIN=${DEPLOYDIR}/ssystem.bin
    else
        echo "ssytem is powerd by lk,we should use signed binary"
        SSYSTEM_BIN=${DEPLOYDIR}/ssystem.bin.sec.signed
    fi

    UNIFIED_BOOT_EXTRA+=" --preload spl:${DEPLOYDIR}/dil.bin.sec.signed
                          --image dil2:${DEPLOYDIR}/dil2.bin
                          --image safety_os:${DEPLOYDIR}/safety.bin
                          --image preloader:${DEPLOYDIR}/preloader.bin
                          --image ddr_fw:${DEPLOYDIR}/ddr_fw.bin
                          --image ddr_init_seq:${DEPLOYDIR}/ddr_init_seq.bin
                          --image ssystem:${SSYSTEM_BIN}
                          --image res:${DEPLOYDIR}/fat_res.img "

    SPECIFIED_PARTITION_PACK+=" --image system_config:${SYSTEM_CONFIG_BIN} "
else
    UNIFIED_BOOT_EXTRA+=" --preload spl:$SIGNED_SPL "
fi

print_warn "------CREATE PARTITION TABLE----------
partition bpt:${BPT_ORIG_FILE}
STORAGE IS :${GLOBAL_BPT_SUFFIX}"

python ./bpttool make_table --input ${BPT_ORIG_FILE} --ab_suffixes "_a,_b" --output_json ${BPT_OUT_FILE} --output_gpt ${BPT_OUT_IMAGE}
test $? -eq 0 || exit 1
print_warn "------CREATE PARTITION TABLE done----------\n"

print_warn "---------------------------------------PACKING PAC----------------STORAGE IS :${GLOBAL_BPT_SUFFIX}------------------------"
DA_COMMON_ARGS=" --product ${CHIPVERSION} --da FDA:$SIGNED_SPL --da OPSIDA:$SIGNED_HANDOVER --da DLOADER:${DEPLOYDIR}/dloader.bin "
SPECIFIED_PARTITION_PACK+=" --allow_empty_partitions "
python ./pactool make_pac_image --output $GLOBAL_OUT --input ${BPT_OUT_FILE} \
        ${DA_COMMON_ARGS} \
        ${UNIFIED_BOOT_EXTRA} \
        ${SPECIFIED_PARTITION_PACK}
test $? -eq 0 || exit 1

./gen_pack_crc ${GLOBAL_OUT}
IMG_SIZE=`stat -c %s ${GLOBAL_OUT}`
IMG_SIZE=`echo "$IMG_SIZE/1024/1024;" | bc`

print_warn "\n File ${GLOBAL_OUT}(size=${IMG_SIZE}Mb) is generated."
