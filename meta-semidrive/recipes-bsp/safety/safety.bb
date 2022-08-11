#
# Recipe for Little kernel

LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

inherit externalsrc

EXTERNALSRC = "${TOPDIR}/../source/lk_safety"
EXTERNALSRC_BUILD = "${EXTERNALSRC}"
PROJECT_BUILDDIR_POSTFIX = "_${MACHAINE_NAME}"
#EXTRA_OEMAKE += "BOARD_VERSION=${BOARD_VERSION}"

PATH_append = \
    ":/tool/gcc_linaro/gcc-arm-none-eabi-7.3.1/bin:/tool/gcc_linaro/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-elf/bin:"
CFLAGS=""

do_configure() {
    echo "skip this step"
}

REPO_SRC_DIR = "${TOPDIR}/../source"
FreeRTOS_DIR = "${REPO_SRC_DIR}/freertos/FreeRTOS"

do_compile[nostamp] += "1"
do_deploy[nostamp] += "1"

do_compile() {
    BPTTOOL=${BSPDIR}/meta-semidrive/scripts/bpttool
    BPTFILE=${BSPDIR}/meta-semidrive/scripts/${MACHAINE_NAME}/global_emmc_only.bpt
    BPT_RES_PARITION_SZ=${PRELOAD_RES_SIZE}
    echo "default PRELOAD_RES_SIZE:"${BPT_RES_PARITION_SZ}
    if [ -n ${PRELOAD_RES_SIZE} -a -f ${BPTFILE} ];then
        res_size=`${BPTTOOL} query_partition --input ${BPTFILE} --label "res" --type size`
        echo "res partition size:"${res_size}
        BPT_RES_PARITION_SZ=${res_size}
        echo "BPT_RES_PARITION_SZ:"${BPT_RES_PARITION_SZ}
    fi

    FreeRTOS_ROOT=${FreeRTOS_DIR} make ${EXTRA_OEMAKE} ${MACHINE_SAFETYARG} PRELOAD_RES_SIZE=${BPT_RES_PARITION_SZ} ${MACHINE_SAFETY} SUPPORT_BOARD_DIAG=${SUPPORT_BOARD_DIAG} VERIFIED_BOOT=${VERIFIED_BOOT}
    FreeRTOS_ROOT=${FreeRTOS_DIR} make ${EXTRA_OEMAKE} ${MACHINE_OSPIHANDOVER} ${MACHINE_OSPIHANDOVERARG}
    FreeRTOS_ROOT=${FreeRTOS_DIR} make ${EXTRA_OEMAKE} ${MACHINE_SAFETYARG} PRELOAD_RES_SIZE=${BPT_RES_PARITION_SZ} dil2 VERIFIED_BOOT=${VERIFIED_BOOT}
}

inherit deploy
do_deploy() {
    install -m 0644 build-${MACHINE_SAFETY}_${CHIPVERSION}_ref/${MACHINE_SAFETY}.bin ${DEPLOYDIR}/safety.bin
    install -m 0644 build-dil2_${CHIPVERSION}_ref/dil2.bin ${DEPLOYDIR}/dil2.bin
    install -m 0644 build-dil2_${CHIPVERSION}_ref/dil2.bin ${DEPLOYDIR}/dil2-unsigned.bin
    install -m 0644 build-${MACHINE_OSPIHANDOVER}_${CHIPVERSION}_ref/${MACHINE_OSPIHANDOVER}.bin ${DEPLOYDIR}/ospihandover.bin
    safety_sign_bin=`find build-${MACHINE_SAFETY}_${CHIPVERSION}_ref -type f -name *.safe.signed`
    if [ -f $safety_sign_bin ];then
        install -m 0644 $safety_sign_bin ${DEPLOYDIR}/safety.bin.signed
    fi
    ospihandover_sign_bin=`find build-${MACHINE_OSPIHANDOVER}_${CHIPVERSION}_ref -type f -name *.safe.signed`
    if [ -f $ospihandover_sign_bin ];then
        install -m 0644 $ospihandover_sign_bin ${DEPLOYDIR}/ospihandover.bin.signed
    fi
}

addtask do_deploy after do_compile
do_buildclean() {
    rm -rf build-${MACHINE_SAFETY}_${CHIPVERSION}_ref
    rm -rf build-${MACHINE_OSPIHANDOVER}_${CHIPVERSION}_ref
}

##FILES_${PN} = "${bindir}/*"
