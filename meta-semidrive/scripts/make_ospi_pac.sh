#!/bin/bash

usage()
{
    echo -e "\nUsage: make_emmc_pac.sh
    Optional parameters: [-y yocto-top] [-b yocto-build-name] [-h] [-k kernel-config-prefix] [-m yocto machine]"
echo "
    * [-y yocto-top-path]:   Yocto top directory, default:$YOCTO_TOP
    * [-b yocto-build]:      Main build dir, default env:$YOCTO_BUILD_DIR
    * [-m yocto-machine]:    Yocto machine [x9h_evb|gx9_evb|x9h_ref], default:$YOCTO_MACHINE
    * [-p Package]:          Global Package [x9hplus_evb], default:$PACKAGE
    * [-s signed-tool]:      Signed tool directory, default $SIGNED_TOOL_DIR/ in current path
    * [-h]:                  This help message
"
}

clean_up()
{
    unset make_pac_help make_pac_error make_pac_flag
    unset usage clean_up
}

source ./sign_tool/sign_helper.sh
export PATH=.:${PWD}/sign_tool/:${PWD}/sign_tool/fec:${PATH}
export LD_LIBRARY_PATH=.:${PWD}/sign_tool/fec:${LD_LIBRARY_PATH}

# get command line options
OLD_OPTIND=$OPTIND

## set default configuration
YOCTO_TOP=`realpath ../..`
YOCTO_BUILD_DIR=${BUILDDIR}
SIGNED_TOOL_DIR=sign_tool
PRODUCT=x9
ATB_SIGN_KEY=./sign_tool/vbmeta/keys/root-key.pem

while getopts "y:s:m:b:p:h" make_pac_flag
do
    case $make_pac_flag in
        y) YOCTO_TOP="$OPTARG";
           ;;
        b) YOCTO_BUILD_DIR="${YOCTO_TOP}/$OPTARG";
           ;;
        m) YOCTO_MACHINE="$OPTARG";
           ;;
        s) SIGNED_TOOL_DIR="$OPTARG";
           ;;
        p) PACKAGE="$OPTARG";
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

if [ "g9x_ref" == $PACKAGE ]; then
    PRODUCT=g9
fi

if [ "g9x_k1" == $PACKAGE ]; then
    PRODUCT=g9
fi

if [ "g9x_ii4" == $PACKAGE ]; then
    PRODUCT=g9
fi

if [ "g9q_ref" == $PACKAGE ]; then
    PRODUCT=g9
fi

if [ "x9h_ref" == $PACKAGE -o "x9h_ref_serdes" == $PACKAGE -o "x9h_refa04_serdes" == $PACKAGE -o "x9h_icl02" == $PACKAGE -o "x9h_ms" == $PACKAGE -o "x9h_classic" == $PACKAGE ]; then
    PRODUCT=x9
fi

if [ "v9f_ref" == $PACKAGE -o "v9f_ref_7inch" == $PACKAGE  -o "v9ts_ref_a" == $PACKAGE -o "v9ts_ref_b" == $PACKAGE -o "v9t_a_ref" == $PACKAGE -o "v9t_b_ref" == $PACKAGE ]; then
    PRODUCT=v9
fi

if [ "x9m_ref_serdes" == $PACKAGE -o "x9m_refa04_serdes" == $PACKAGE -o "x9m_ref" == $PACKAGE -o "x9m_ms" == $PACKAGE ]; then
    PRODUCT=x9
fi

if [ "d9_ref" == $PACKAGE ]; then
    PRODUCT=d9
fi

if [ "d9lite_ref" == $PACKAGE ]; then
    PRODUCT=d9
fi

if [[ "x$PACKAGE" == "xd9plus_ref"* ]]; then
    PRODUCT=d9
fi

if [ "x9e_ref" == $PACKAGE -o "x9e_ms" == $PACKAGE ]; then
    PRODUCT=x9
fi

if [ "x9hplus_evb" == $PACKAGE ]; then
    PRODUCT=x9
    YOCTO_MACHINE=x9h-plus_evb_ivi
fi

if [ "bf200" == $PACKAGE ]; then
    PRODUCT=bf200
fi

echo -e " TOP directory  : " $YOCTO_TOP
echo -e " Build directory: " $YOCTO_BUILD_DIR
echo -e " Build Package  : " $PACKAGE
echo -e " Build machine  : " $YOCTO_MACHINE

SIGNED_SPL=${PACKAGE}/prebuilts/spl.bin.signed.rsa1024
SIGNED_HANDOVER=${PACKAGE}/prebuilts/ospihandover.bin.safe.signed
UNIFIED_BOOT_EXTRA=

if [ ! -e $SIGNED_TOOL_DIR ]; then
    echo -e "\n ERROR - No $SIGNED_TOOL_DIR/ found"
    echo -e "\n Use -s to assign a sign tool directory"
    make_ospi_flag_sign='false'
else
    echo -e " Signed tool path is "`realpath $SIGNED_TOOL_DIR`
    make_ospi_flag_sign='true'
fi

OSPI_OUT=$YOCTO_BUILD_DIR/ospi_safety.pac

## check the necessary input files
YOCTO_IMAGE_DIR=${YOCTO_BUILD_DIR}/tmp/deploy/images/${YOCTO_MACHINE}
if [ ! -e $YOCTO_IMAGE_DIR ]; then
    echo -e "\n ERROR - No yocto Image found in the path" $YOCTO_IMAGE_DIR
    echo -e "\n use 'bitbake safety' command to build"
    exit 1
fi

if [ ! -e $YOCTO_IMAGE_DIR/safety.bin ]; then
    echo -e "\n ERROR - No safety bin found in the path" $YOCTO_IMAGE_DIR
    echo -e "\n try 'bitbake safety' command to build"
    exit 1
fi

mkdir out -p
if [ "bf200" == $PACKAGE ]; then
python ./gen_sfs_binary.py --json ${PACKAGE}/gd25_lb256e_low.json --out out/sfs.img
else
python ./gen_sfs_binary.py --json ${PACKAGE}/mt35_octal_ouput_fast_read.json --out out/sfs.img
fi

function process_unified_boot_file()
{
    PROJECT_NAME=${PACKAGE##*_}
    if [[ x$PROJECT_NAME == x"ref" ]];then
        PROJECT_NAME=default
    fi

    ./sign_tool/run_sign_safe -i ${YOCTO_IMAGE_DIR}/dil.bin -l 0x100000 -e 0x100000 -k ${ATB_SIGN_KEY}
    cp ${YOCTO_IMAGE_DIR}/dil.bin.safe.signed out/safety.img

    ./sign_tool/run_sign_sec -i ${YOCTO_IMAGE_DIR}/ssystem.bin -k ${ATB_SIGN_KEY}

    ls -al ${YOCTO_IMAGE_DIR}/ddr_init_seq.bin

    UNIFIED_BOOT_EXTRA+=" --image dil:${YOCTO_IMAGE_DIR}/dil.bin.safe.signed
                          --image dil_bak:${YOCTO_IMAGE_DIR}/dil.bin.safe.signed
                          --image dil2:${YOCTO_IMAGE_DIR}/dil2.bin
                          --image safety_os:${YOCTO_IMAGE_DIR}/safety.bin
                          --image preloader:${YOCTO_IMAGE_DIR}/preloader.bin
                          --image ddr_fw:${YOCTO_IMAGE_DIR}/ddr_fw.bin
                          --image ddr_init_seq:${YOCTO_IMAGE_DIR}/ddr_init_seq.bin
                          --image ssystem:${YOCTO_IMAGE_DIR}/ssystem.bin.sec.signed "

    return 1
}

function create_res_image()
{
    bpt_file=$1
    res_root=$2
    out_name=$3

    RES_IMAGE_SIZE=$(get_image_max_size "${BPT_ORIG_FILE}" "res" "hash" )
    RES_IMAGE_SIZE=$((RES_IMAGE_SIZE/1048576))

    mkdir -p ${res_root}/early_app
    cp -r ../../source/lk_safety/res/early_app/BootAnimation ${res_root}/early_app/
    cp -r ../../source/lk_safety/res/early_app/audio ${res_root}/early_app/
    ./dir2fat.sh -f -F 16 -S 512 ${out_name} ${RES_IMAGE_SIZE} ${res_root};
}

if test $make_ospi_flag_sign; then
    process_unified_boot_file

    if [[ $? -eq 0 ]];then
        cp ${YOCTO_IMAGE_DIR}/safety.bin.signed out/safety.img
    fi

    SIGNED_SAFETY=out/safety.img

    ./sign_tool/run_sign_sec -i ${YOCTO_IMAGE_DIR}/spl.bin -k ${ATB_SIGN_KEY}
    SIGNED_SPL=${YOCTO_IMAGE_DIR}/spl.bin.sec.signed
    SIGNED_HANDOVER=${YOCTO_IMAGE_DIR}/ospihandover.bin.signed
else
    SIGNED_SAFETY=${PACKAGE}/prebuilts/safety.img
    echo -e " Use default signed safety.img in ${PACKAGE}/prebuilit/"
    SIGNED_SPL=${PACKAGE}/prebuilts/spl.bin.signed.rsa1024
    SIGNED_HANDOVER=${PACKAGE}/prebuilts/ospihandover.bin.safe.signed
fi

DA_COMMON_ARGS=" --product ${PRODUCT} --da FDA:$SIGNED_SPL --da OPSIDA:$SIGNED_HANDOVER --da DLOADER:${YOCTO_IMAGE_DIR}/dloader.bin "
SPECIFIED_PARTITION_PACK+=" --preload sfs:out/sfs.img  --allow_empty_partitions "

BPT_ORIG_FILE=${PACKAGE}/ospi.bpt
HASH_ALG=sha256
KEY_FILE="./sign_tool/vbmeta/keys/vbmeta-user-key.pem"
KEY_META="./sign_tool/vbmeta/keys/vbmeta-user-rsa2048whitsha256-romtest.cer.chain"
VBMETA_SIGNED_ALG=SHA256_RSA2048
VBMETA_SIGNED_ARGS="--algorithm ${VBMETA_SIGNED_ALG} --key ${KEY_FILE} --public_key_meta ${KEY_META}"
VBMTEA_DESCIPTOR=

#sign dloader
#image_add_hash_footer_no_bpt "${YOCTO_IMAGE_DIR}/dloader.bin" "dloader" "262144" "${HASH_ALG}" "${VBMETA_SIGNED_ARGS}"

VBMTEA_DESCIPTOR+=" ${YOCTO_IMAGE_DIR}/dil2.bin:dil2:${HASH_ALG}:hash
                    ${YOCTO_IMAGE_DIR}/ddr_init_seq.bin:ddr_init_seq:${HASH_ALG}:hash
                    ${YOCTO_IMAGE_DIR}/ddr_fw.bin:ddr_fw:${HASH_ALG}:hash
                    ${YOCTO_IMAGE_DIR}/preloader.bin:preloader:${HASH_ALG}:hash
                    ${YOCTO_IMAGE_DIR}/safety.bin:safety_os:${HASH_ALG}:hash"

# for ssystem warm boot, use avb2.0 to verify ssystem image, so add its info to vbmeta
VBMTEA_DESCIPTOR+=" ${YOCTO_IMAGE_DIR}/ssystem.bin.sec.signed:ssystem:${HASH_ALG}:hash "

python ./bpttool make_table --input ${BPT_ORIG_FILE} --ab_suffixes "_a,_b" --output_json out/ospi_bak_output.bpt --output_gpt out/ospi_bak_partition.img
test $? -eq 0 || exit 1

if [ "g9x_ref" == $PACKAGE -o "g9x_ii4" == $PACKAGE -o "g9x_k1" == $PACKAGE ]; then
    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/g9x/projects/${PROJECT_NAME}/system_config.bin
    VBMTEA_DESCIPTOR+=" ${SYSTEM_CONFIG_BIN}:system_config:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/sdpe.bin:sdpe_fw:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/sdpe_cfg.bin:routing-table:${HASH_ALG}:hash"

    SPECIFIED_PARTITION_PACK+=" --image sdpe_fw:${YOCTO_IMAGE_DIR}/sdpe.bin
                                --image routing-table:${YOCTO_IMAGE_DIR}/sdpe_cfg.bin
                                --image system_config:${SYSTEM_CONFIG_BIN} "

elif [ "g9q_ref" == $PACKAGE ]; then
    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/g9q/projects/${PROJECT_NAME}/system_config.bin
    VBMTEA_DESCIPTOR+=" ${SYSTEM_CONFIG_BIN}:system_config:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/sdpe.bin:sdpe_fw:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/sdpe_cfg.bin:routing-table:${HASH_ALG}:hash"

    SPECIFIED_PARTITION_PACK+=" --image sdpe_fw:${YOCTO_IMAGE_DIR}/sdpe.bin
                                --image routing-table:${YOCTO_IMAGE_DIR}/sdpe_cfg.bin
                                --image system_config:${SYSTEM_CONFIG_BIN} "
elif [ "v9f_ref" == $PACKAGE -o "v9f_ref_7inch" == $PACKAGE ]; then
    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/v9f/projects/default/system_config.bin
    if [ "v9f_ref_7inch" == $PACKAGE ]; then
        SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/v9f/projects/default/system_config_7inch.bin
    fi

    VBMTEA_DESCIPTOR+=" ${SYSTEM_CONFIG_BIN}:system_config:${HASH_ALG}:hash"
    SPECIFIED_PARTITION_PACK+=" --image system_config:${SYSTEM_CONFIG_BIN} "

elif [ "v9ts_ref_a" == $PACKAGE -o "v9ts_ref_b" == $PACKAGE ]; then
    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/v9ts/projects/default/side_a/system_config.bin
    if [ "v9ts_ref_b" == $PACKAGE  ]; then
        SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/v9ts/projects/default/side_b/system_config.bin
    fi
    VBMTEA_DESCIPTOR+=" ${SYSTEM_CONFIG_BIN}:system_config:${HASH_ALG}:hash"

    SPECIFIED_PARTITION_PACK+=" --image system_config:${SYSTEM_CONFIG_BIN} "

elif [ "x9h_ref" == $PACKAGE -o "x9h_ref_cluster" == $PACKAGE  -o "x9h_ref_cluster_serdes" == $PACKAGE \
        -o "x9h_ref_serdes" == $PACKAGE \
        -o "x9h_refa04_serdes" == $PACKAGE \
        -o "x9h_ref_controlpanel" == $PACKAGE \
        -o "x9h_ref_controlpanel_serdes" == $PACKAGE \
        -o "x9h_ref_bt" == $PACKAGE \
        -o "x9h_icl02" == $PACKAGE \
        -o "x9h_ms" == $PACKAGE \
        -o "x9h_classic" == $PACKAGE ]; then

    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/x9_high/projects/default/system_config.bin
    if [ ! -e ${SYSTEM_CONFIG_BIN} ];then
        SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/x9_high/projects/$YOCTO_MACHINE/system_config.bin;
    fi

    if [ "x9h_ref_cluster_serdes" == $YOCTO_MACHINE -o "x9h_ref_controlpanel_serdes" == $YOCTO_MACHINE \
           -o  "x9h_ref_serdes" == $YOCTO_MACHINE -o  "x9h_refa04_serdes" == $YOCTO_MACHINE -o  "x9h_icl02" == $YOCTO_MACHINE ]; then
        SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/x9_high/projects/controlpanel/system_config.bin
    fi

    if [ "x9h_icl02" == $YOCTO_MACHINE ]; then
        SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/x9_high/projects/icl02/system_config.bin
    fi

    if [ "x9h_ms" == $YOCTO_MACHINE ]; then
        SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/x9_high/projects/ms_serdes/system_config.bin
    fi

    if [ "x9h_classic" == $YOCTO_MACHINE ]; then
        SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/x9_high/projects/classic/system_config.bin
    fi

    create_res_image ${BPT_ORIG_FILE} ${YOCTO_IMAGE_DIR}/res_img ${YOCTO_IMAGE_DIR}/fat_res.img
    VBMTEA_DESCIPTOR+=" ${SYSTEM_CONFIG_BIN}:system_config:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/fat_res.img:res:${HASH_ALG}:hash"

    SPECIFIED_PARTITION_PACK+=" --image res:${YOCTO_IMAGE_DIR}/fat_res.img
                                --image system_config:${SYSTEM_CONFIG_BIN} "

elif [ "x9m_ref_serdes" == $PACKAGE -o "x9m_refa04_serdes" == $PACKAGE ]; then
    create_res_image ${BPT_ORIG_FILE} ${YOCTO_IMAGE_DIR}/res_img ${YOCTO_IMAGE_DIR}/fat_res.img
    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/x9_mid/projects/serdes/system_config.bin
    VBMTEA_DESCIPTOR+=" ${SYSTEM_CONFIG_BIN}:system_config:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/fat_res.img:res:${HASH_ALG}:hash"

    SPECIFIED_PARTITION_PACK+=" --image res:${YOCTO_IMAGE_DIR}/fat_res.img
                                --image system_config:${SYSTEM_CONFIG_BIN} "

elif [ "x9m_ref" == $PACKAGE ]; then
    create_res_image ${BPT_ORIG_FILE} ${YOCTO_IMAGE_DIR}/res_img ${YOCTO_IMAGE_DIR}/fat_res.img
    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/x9_mid/projects/default/system_config.bin
    VBMTEA_DESCIPTOR+=" ${SYSTEM_CONFIG_BIN}:system_config:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/fat_res.img:res:${HASH_ALG}:hash"

    SPECIFIED_PARTITION_PACK+=" --image res:${YOCTO_IMAGE_DIR}/fat_res.img
                                --image system_config:${SYSTEM_CONFIG_BIN} "

elif [ "xd9_ref" == "x$PACKAGE" ]; then
    create_res_image ${BPT_ORIG_FILE} ${YOCTO_IMAGE_DIR}/res_img ${YOCTO_IMAGE_DIR}/fat_res.img
    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/d9/projects/default/system_config.bin
    if [ ! -e ${SYSTEM_CONFIG_BIN} ];then
        SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/d9/projects/$YOCTO_MACHINE/system_config.bin;
    fi

    VBMTEA_DESCIPTOR+=" ${SYSTEM_CONFIG_BIN}:system_config:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/fat_res.img:res:${HASH_ALG}:hash"

    SPECIFIED_PARTITION_PACK+=" --image res:${YOCTO_IMAGE_DIR}/fat_res.img
                                --image system_config:${SYSTEM_CONFIG_BIN} "

elif [ "xd9lite_ref" == "x$PACKAGE" ]; then
    create_res_image ${BPT_ORIG_FILE} ${YOCTO_IMAGE_DIR}/res_img ${YOCTO_IMAGE_DIR}/fat_res.img
    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/d9lite/projects/default/system_config.bin
    if [ ! -e ${SYSTEM_CONFIG_BIN} ];then
        SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/d9lite/projects/$YOCTO_MACHINE/system_config.bin;
    fi

    VBMTEA_DESCIPTOR+=" ${SYSTEM_CONFIG_BIN}:system_config:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/fat_res.img:res:${HASH_ALG}:hash"

    SPECIFIED_PARTITION_PACK+=" --image res:${YOCTO_IMAGE_DIR}/fat_res.img
                                --image system_config:${SYSTEM_CONFIG_BIN} "

elif [[ "x$PACKAGE" == "xd9plus_ref"* ]]; then
    create_res_image ${BPT_ORIG_FILE} ${YOCTO_IMAGE_DIR}/res_img ${YOCTO_IMAGE_DIR}/fat_res.img
    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/d9plus/projects/default/system_config.bin
 
    if [ ! -e ${SYSTEM_CONFIG_BIN} ];then
        SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/d9plus/projects/$YOCTO_MACHINE/system_config.bin;
    fi

    if [ ! -e ${SYSTEM_CONFIG_BIN} ];then
        SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/d9plus/projects/d9plus_ref/system_config.bin;
    fi

    VBMTEA_DESCIPTOR+=" ${SYSTEM_CONFIG_BIN}:system_config:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/fat_res.img:res:${HASH_ALG}:hash"

    SPECIFIED_PARTITION_PACK+=" --image res:${YOCTO_IMAGE_DIR}/fat_res.img
                                --image system_config:${SYSTEM_CONFIG_BIN} "

elif [ "x9h_evb" == $PACKAGE -o "x9h_evb_cluster" == $PACKAGE  ]; then
    create_res_image ${BPT_ORIG_FILE} ${YOCTO_IMAGE_DIR}/res_img ${YOCTO_IMAGE_DIR}/fat_res.img
    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/x9_high/projects/evb/system_config.bin
    VBMTEA_DESCIPTOR+=" ${SYSTEM_CONFIG_BIN}:system_config:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/fat_res.img:res:${HASH_ALG}:hash"

    SPECIFIED_PARTITION_PACK+=" --image res:${YOCTO_IMAGE_DIR}/fat_res.img
                                --image system_config:${SYSTEM_CONFIG_BIN} "

elif [ "x9hplus_evb" == $PACKAGE ]; then
    create_res_image ${BPT_ORIG_FILE} ${YOCTO_IMAGE_DIR}/res_img ${YOCTO_IMAGE_DIR}/fat_res.img
    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/x9_high-plus/projects/evb/system_config.bin
    VBMTEA_DESCIPTOR+=" ${SYSTEM_CONFIG_BIN}:system_config:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/fat_res.img:res:${HASH_ALG}:hash"

    SPECIFIED_PARTITION_PACK+=" --image res:${YOCTO_IMAGE_DIR}/fat_res.img
                                --image system_config:${SYSTEM_CONFIG_BIN} "

elif [ "v9t_a_ref" == $PACKAGE -o "v9t_b_ref" == $PACKAGE ]; then
    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/v9t/projects/serdes/side_a/system_config.bin
    if [ "v9t_b_ref" == $PACKAGE  ]; then
        SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/v9t/projects/serdes/side_b/system_config.bin
    fi

    VBMTEA_DESCIPTOR+=" ${SYSTEM_CONFIG_BIN}:system_config:${HASH_ALG}:hash"
    SPECIFIED_PARTITION_PACK+=" --image system_config:${SYSTEM_CONFIG_BIN} "

elif [ "x9m_ms" == $PACKAGE ]; then
    create_res_image ${BPT_ORIG_FILE} ${YOCTO_IMAGE_DIR}/res_img ${YOCTO_IMAGE_DIR}/fat_res.img
    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/x9_mid/projects/ms_serdes/system_config.bin

    VBMTEA_DESCIPTOR+=" ${SYSTEM_CONFIG_BIN}:system_config:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/fat_res.img:res:${HASH_ALG}:hash"

    SPECIFIED_PARTITION_PACK+=" --image res:${YOCTO_IMAGE_DIR}/fat_res.img
                                --image system_config:${SYSTEM_CONFIG_BIN} "
elif [ "x9e_ms" == $PACKAGE ]; then
    create_res_image ${BPT_ORIG_FILE} ${YOCTO_IMAGE_DIR}/res_img ${YOCTO_IMAGE_DIR}/fat_res.img
    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/x9_eco/projects/ms_serdes/system_config.bin
    VBMTEA_DESCIPTOR+=" ${SYSTEM_CONFIG_BIN}:system_config:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/fat_res.img:res:${HASH_ALG}:hash"

    SPECIFIED_PARTITION_PACK+=" --image res:${YOCTO_IMAGE_DIR}/fat_res.img
                                --image system_config:${SYSTEM_CONFIG_BIN} "
elif [ "x9e_ref" == $PACKAGE ]; then
    create_res_image ${BPT_ORIG_FILE} ${YOCTO_IMAGE_DIR}/res_img ${YOCTO_IMAGE_DIR}/fat_res.img
    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/x9_eco/projects/serdes/system_config.bin
    VBMTEA_DESCIPTOR+=" ${SYSTEM_CONFIG_BIN}:system_config:${HASH_ALG}:hash
                        ${YOCTO_IMAGE_DIR}/fat_res.img:res:${HASH_ALG}:hash"

    SPECIFIED_PARTITION_PACK+=" --image res:${YOCTO_IMAGE_DIR}/fat_res.img
                                --image system_config:${SYSTEM_CONFIG_BIN} "
elif [ "bf200" == $PACKAGE ]; then
    SYSTEM_CONFIG_BIN=${YOCTO_TOP}/source/lk_customize/chipcfg/generate/bf200/projects/${PROJECT_NAME}/system_config.bin
    VBMTEA_DESCIPTOR=" ${SYSTEM_CONFIG_BIN}:system_config:${HASH_ALG}:hash"
	VBMTEA_DESCIPTOR+=" ${YOCTO_IMAGE_DIR}/ssystem.bin.sec.signed:ssystem:${HASH_ALG}:hash "
    VBMTEA_DESCIPTOR+=" ${YOCTO_IMAGE_DIR}/dil2.bin:dil2:${HASH_ALG}:hash
                    ${YOCTO_IMAGE_DIR}/ddr_init_seq.bin:ddr_init_seq:${HASH_ALG}:hash
                    ${YOCTO_IMAGE_DIR}/ddr_fw.bin:ddr_fw:${HASH_ALG}:hash
                    ${YOCTO_IMAGE_DIR}/safety.bin:safety_os:${HASH_ALG}:hash"

    SPECIFIED_PARTITION_PACK+=" --image system_config:${SYSTEM_CONFIG_BIN} "
else
    create_res_image ${BPT_ORIG_FILE} ${YOCTO_IMAGE_DIR}/res_img ${YOCTO_IMAGE_DIR}/fat_res.img
    UNIFIED_BOOT_EXTRA=
    VBMTEA_DESCIPTOR+=" ${YOCTO_IMAGE_DIR}/fat_res.img:res:${HASH_ALG}:hash"

    SPECIFIED_PARTITION_PACK+=" --image os:${SIGNED_SAFETY}
                                --image os_bak:${SIGNED_SAFETY}
                                --image res:${YOCTO_IMAGE_DIR}/fat_res.img "
fi

VBMETA_IMAGE_OSPI=${YOCTO_IMAGE_DIR}/vbmeta-ospi.img
VBMETA_IMAGE_EMMC=${YOCTO_IMAGE_DIR}/vbmeta-emmc.img
VBMETA_IMAGE_FINAL=${YOCTO_IMAGE_DIR}/vbmeta-final.img
VBMTEA_DESCIPTOR=$(images_add_footer ${BPT_ORIG_FILE} "${VBMTEA_DESCIPTOR}")
echo "OSPI VBMTEA_DESCIPTOR:"${VBMTEA_DESCIPTOR}

make_vbmeta_image "${VBMTEA_DESCIPTOR}" "${VBMETA_SIGNED_ARGS}" "${VBMETA_IMAGE_OSPI}" || exit 1
if [ "bf200" == $PACKAGE ]; then
	VBMTEA_DESCIPTOR=" --include_descriptors_from_image ${VBMETA_IMAGE_OSPI}"
else
    if [ ! -e ${VBMETA_IMAGE_EMMC} ];then
        echo "please make emmc pack in the first!"
        exit -1
    fi

    # All descriptor are in VBMETA_IMAGE_OSPI and VBMETA_IMAGE_EMMC
    VBMTEA_DESCIPTOR=" --include_descriptors_from_image ${VBMETA_IMAGE_OSPI}
                       --include_descriptors_from_image ${VBMETA_IMAGE_EMMC} "
fi

echo "FINAL VBMTEA_DESCIPTOR:"${VBMTEA_DESCIPTOR}
make_vbmeta_image "${VBMTEA_DESCIPTOR}" "${VBMETA_SIGNED_ARGS}" "${VBMETA_IMAGE_FINAL}" || exit 1

SPECIFIED_PARTITION_PACK+=" --image vbmeta:${VBMETA_IMAGE_FINAL} "
python ./pactool make_pac_image --output ${OSPI_OUT} --input out/ospi_bak_output.bpt \
        ${SPECIFIED_PARTITION_PACK} \
        ${DA_COMMON_ARGS} \
        ${UNIFIED_BOOT_EXTRA}

test $? -eq 0 || exit 1

./gen_pack_crc ${OSPI_OUT}
echo -e "\n File  ${OSPI_OUT} is generated"
