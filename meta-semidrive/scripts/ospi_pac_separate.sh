if [ -f ${SD_TOPDIR}/.project.conf ];then
    source ${SD_TOPDIR}/.project.conf
fi

#DEPLOYDIR=${SD_TOPDIR}/out/${MACHINE_DEPLOYDIR}/binary

#SD_DIR_PROJECT_CONFIG=${SD_TOPDIR}/source/chipcfg/generate/${CHIPVERSION}/projects/${MACHINE_PROJECT}
BPT_ORIG_FILE=${SD_DIR_PROJECT_CONFIG}/pack_cfg/ospi.bpt

if [ -d ${SD_TOPDIR}/build/scripts/sign_tool ];then
    SD_DIR_BUILD_SCRIPTS=${SD_TOPDIR}/build/scripts
else
    SD_DIR_BUILD_SCRIPTS=${SD_TOPDIR}/meta-semidrive/scripts
fi
echo "SD_DIR_BUILD_SCRIPTS="$SD_DIR_BUILD_SCRIPTS"" >> .project.conf

ATB_SIGN_KEY=${SD_DIR_BUILD_SCRIPTS}/sign_tool/vbmeta/keys/root-key.pem

source ${SD_DIR_BUILD_SCRIPTS}/sign_tool/sign_helper.sh
export PATH=.:${SD_DIR_BUILD_SCRIPTS}/sign_tool/:${SD_DIR_BUILD_SCRIPTS}/sign_tool/fec:${PATH}
export LD_LIBRARY_PATH=.:${SD_DIR_BUILD_SCRIPTS}/sign_tool/fec:${LD_LIBRARY_PATH}

OSPI_OUT=$DEPLOYDIR/ospi_safety.pac
OSPI_OUT=`cd ${DEPLOYDIR}/../images;pwd -P`
OSPI_OUT=${OSPI_OUT}/ospi_safety.pac

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

create_res_image ${BPT_ORIG_FILE} ${DEPLOYDIR}/res_img ${DEPLOYDIR}/fat_res.img

python ${SD_DIR_BUILD_SCRIPTS}/gen_sfs_binary.py --json \
    ${SD_DIR_PROJECT_CONFIG}/pack_cfg/mt35_octal_ouput_fast_read_gd25q.json \
    --out $DEPLOYDIR/sfs.img

${SD_DIR_BUILD_SCRIPTS}/sign_tool/run_sign_safe -i ${DEPLOYDIR}/dil.bin -l 0x100000 -e 0x100000 -k ${ATB_SIGN_KEY}
cp ${DEPLOYDIR}/dil.bin.safe.signed ${DEPLOYDIR}/safety.img

${SD_DIR_BUILD_SCRIPTS}/sign_tool/run_sign_sec -i ${DEPLOYDIR}/spl.bin -k ${ATB_SIGN_KEY}
SIGNED_SPL=${DEPLOYDIR}/spl.bin.sec.signed

${SD_DIR_BUILD_SCRIPTS}/sign_tool/run_sign_sec -i ${DEPLOYDIR}/ssystem.bin -l 0x140000 -e 0x140000 -k ${ATB_SIGN_KEY}

python ${SD_DIR_BUILD_SCRIPTS}/bpttool make_table --input ${BPT_ORIG_FILE} --ab_suffixes "_a,_b" --output_json $DEPLOYDIR/GPT_ospi_output.bpt --output_gpt $DEPLOYDIR/ospi_bak_output.img
if [ ! -e $DEPLOYDIR/GPT_ospi_output.bpt ]; then echo "gen $DEPLOYDIR/GPT_ospi_output.bpt failed."; exit 1 ; fi

SIGNED_HANDOVER=${DEPLOYDIR}/ospihandover.bin.signed

SPECIFIED_PARTITION_PACK+=" --image system_config:${SD_DIR_PROJECT_CONFIG}/system_config.bin "

SPECIFIED_PARTITION_PACK+=" --image res:${DEPLOYDIR}/fat_res.img "

DA_COMMON_ARGS=" --product ${CHIPVERSION} \
            --da FDA:${DEPLOYDIR}/spl.bin.sec.signed \
            --da OPSIDA:${DEPLOYDIR}/ospihandover.bin.signed \
            --da DLOADER:${DEPLOYDIR}/dloader.bin "

SPECIFIED_PARTITION_PACK+=" --preload sfs:$DEPLOYDIR/sfs.img  --allow_empty_partitions "

UNIFIED_BOOT_EXTRA=" --image dil:${DEPLOYDIR}/dil.bin.safe.signed
                        --image dil_bak:${DEPLOYDIR}/dil.bin.safe.signed
                        --image dil2:${DEPLOYDIR}/dil2.bin
                        --image safety_os:${DEPLOYDIR}/safety.bin
                        --image preloader:${DEPLOYDIR}/preloader.bin
                        --image ddr_fw:${DEPLOYDIR}/ddr_fw.bin
                        --image ddr_init_seq:${DEPLOYDIR}/ddr_init_seq.bin
                        --image ssystem:${DEPLOYDIR}/ssystem.bin.sec.signed
                        "

python ${SD_DIR_BUILD_SCRIPTS}/pactool make_pac_image --output ${OSPI_OUT} --input $DEPLOYDIR/GPT_ospi_output.bpt \
        ${SPECIFIED_PARTITION_PACK} \
        ${DA_COMMON_ARGS} \
        ${UNIFIED_BOOT_EXTRA}

test $? -eq 0 || exit 1

./gen_pack_crc ${OSPI_OUT}
IMG_SIZE=`stat -c %s ${OSPI_OUT}`
IMG_SIZE=`echo "$IMG_SIZE/1024/1024;" | bc`

echo -e "\n File ${OSPI_OUT}(size=${IMG_SIZE}Mb) is generated."
