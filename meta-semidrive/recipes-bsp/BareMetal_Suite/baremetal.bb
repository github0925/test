#
# Recipe for Little kernel

LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

inherit externalsrc

EXTERNALSRC = "${TOPDIR}/../source/BareMetal_Suite"
EXTERNALSRC_BUILD = "${EXTERNALSRC}"

EXTRA_OEMAKE = ""

PATH_append = \
    ":/tool/gcc_linaro/gcc-arm-none-eabi-7.3.1/bin:/tool/gcc_linaro/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-elf/bin:"
CFLAGS=""

do_configure() {
    echo "skip this step"
}

REPO_SRC_DIR = "${TOPDIR}/../source"

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

    make CFG=t_loader clean -j2
    if [ x${BAREMETAL_TGT} != x"" ];then
        make CFG=t_loader unified_boot ${MACHINE_BAREMETALARG} PRELOAD_RES_SIZE=${BPT_RES_PARITION_SZ} TGT=${BAREMETAL_TGT} VERIFIED_BOOT=${VERIFIED_BOOT}
    else
        make CFG=t_loader unified_boot ${MACHINE_BAREMETALARG} VERIFIED_BOOT=${VERIFIED_BOOT}
    fi
}

inherit deploy
do_deploy() {
    if [ x${BAREMETAL_TGT} != x"" ];then
        install -m 0644 build/kunlun/${BAREMETAL_TGT}/${BAREMETAL_TGT}_t_loader.elf.bin ${DEPLOYDIR}/dil.bin
    else
        install -m 0644 build/kunlun/safe/safe_t_loader.elf.bin ${DEPLOYDIR}/dil.bin
    fi
}

addtask do_deploy after do_compile
do_buildclean() {
    make clean
    rm -rf build/kunlun/safe/*
}

##FILES_${PN} = "${bindir}/*"
