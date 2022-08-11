#
# Recipe for Little kernel

LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

inherit externalsrc

EXTERNALSRC = "${TOPDIR}/../source/lk_customize"
EXTERNALSRC_BUILD = "${EXTERNALSRC}"

EXTRA_OEMAKE = ""

PATH_append = \
    ":/tool/gcc_linaro/gcc-arm-none-eabi-7.3.1/bin:/tool/gcc_linaro/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-elf/bin:"
CFLAGS=""

def get_sd_lk_toolchain(d):
    origenv = d.getVar("BB_ORIGENV", False)
    lk_toolchain = origenv.getVar("SD_LK_TOOLCHAIN", False)
    return lk_toolchain

lk_toolchain_path := "${@get_sd_lk_toolchain(d)}"

PATH_append = "${lk_toolchain_path}"

MACHAINE_BOOTLOADER_TYPE ?= "lk"

do_configure() {
    echo "skip this step"
}

EXTRA_OEMAKE = "${MACHINE_BOOT_EXTRA_OPTION}"
do_compile() {
    if [ "x${MACHINE_SPL}" != "x" ]; then
        make ${MACHINE_SPLARG} VERIFIED_BOOT=${VERIFIED_BOOT} ${MACHINE_SPL}
    fi
    if [ "x${MACHINE_DLOADER}" != "x" ]; then
        make ${MACHINE_DLOADERARG} VERIFIED_BOOT=${VERIFIED_BOOT} ${MACHINE_DLOADER}
    fi
    if [ "x${MACHINE_PRELOADER}" != "x" ]; then
        make ${MACHINE_PRELOADERARG} VERIFIED_BOOT=${VERIFIED_BOOT} ${MACHINE_PRELOADER}
    fi

    if [ "x${MACHINE_BOOTLOADER}" != "x" -a "${MACHAINE_BOOTLOADER_TYPE}" = "lk" ]; then
        bbplain "------ make ${MACHINE_BOOTLOADERARG} VERIFIED_BOOT=${VERIFIED_BOOT} ${MACHINE_BOOTLOADER}"
        make ${MACHINE_BOOTLOADERARG} VERIFIED_BOOT=${VERIFIED_BOOT} ${MACHINE_BOOTLOADER}
    fi
}

do_compile[nostamp] += "1"
do_deploy[nostamp] += "1"
AP_STR=""
inherit deploy
do_deploy() {
    if [ "x${MACHINE}" == "xd9plus_ref_ap1" ];then
        AP_STR="_ap1";
    elif [ "x${MACHINE}" == "xd9plus_ref_ap2" ];then
        AP_STR="_ap2";
    else
        AP_STR="";
    fi
    if [ -f ${TOPDIR}/../meta-semidrive/scripts/${MACHINE}${AP_STR}/ssystem.bin ]; then
        install -m 0644 ${TOPDIR}/../meta-semidrive/scripts/${MACHINE}${AP_STR}/ssystem.bin ${DEPLOYDIR}/ssystem.bin
    fi

    if [ -f ${TOPDIR}/../source/chipcfg/generate/${CHIPVERSION}/projects/${MACHAINE_NAME}/pack_cfg/prebuilts/ssystem.bin ]; then
        install -m 0644 ${TOPDIR}/../source/chipcfg/generate/${CHIPVERSION}/projects/${MACHAINE_NAME}/pack_cfg/prebuilts/ssystem.bin ${DEPLOYDIR}/ssystem.bin
    fi

    if [ "x${MACHINE_SPL}" != "x" ]; then
        install -m 0644 build-${MACHINE_SPL}_${CHIPVERSION}_ref/lk.bin ${DEPLOYDIR}/spl.bin
        install -m 0644 build-${MACHINE_SPL}_${CHIPVERSION}_ref/ddr_fw.bin ${DEPLOYDIR}/ddr_fw.bin
        install -m 0644 build-${MACHINE_SPL}_${CHIPVERSION}_ref/ddr_init_seq.bin ${DEPLOYDIR}/ddr_init_seq.bin
    fi
    if [ "x${MACHINE_DLOADER}" != "x" ]; then
        install -m 0644 build-${MACHINE_DLOADER}_${CHIPVERSION}_ref/lk.bin ${DEPLOYDIR}/dloader.bin
    fi
    if [ "x${MACHINE_PRELOADER}" != "x" ]; then
        install -m 0644 build-${MACHINE_PRELOADER}_${CHIPVERSION}_ref/lk.bin ${DEPLOYDIR}/preloader.bin
    fi

    if [ "x${MACHINE_BOOTLOADER}" != "x" -a "${MACHAINE_BOOTLOADER_TYPE}" = "lk" ]; then
        install -m 0644 build-${MACHINE_BOOTLOADER}_${CHIPVERSION}_ref/lk.bin ${DEPLOYDIR}/ivi_bootloader.bin
    fi
}

addtask do_deploy after do_compile
do_buildclean() {
    rm -rf build-*
}

##FILES_${PN} = "${bindir}/*"
