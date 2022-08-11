#
# Recipe for Little kernel

LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

inherit externalsrc

EXTERNALSRC = "${TOPDIR}/../source/ssdk"
EXTERNALSRC_BUILD = "${EXTERNALSRC}"

EXTRA_OEMAKE = ""

PATH_append = \
    ":/tool/gcc_linaro/gcc-arm-none-eabi-7.3.1/bin:/tool/gcc_linaro/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-elf/bin:"
CFLAGS=""

def get_sd_ssdk_toolchain(d):
    origenv = d.getVar("BB_ORIGENV", False)
    ssdk_toolchain = origenv.getVar("SD_ssdk_TOOLCHAIN", False)
    return ssdk_toolchain

ssdk_toolchain_path := "${@get_sd_ssdk_toolchain(d)}"

PATH_append = "${ssdk_toolchain_path}"

do_configure() {
    echo "skip this step"
}

EXTRA_OEMAKE = "${MACHINE_BOOT_EXTRA_OPTION}"
do_compile() {
    bbplain "[ssdk]make ${SD_SSDK_SAF}"
    make ${SD_SSDK_SAF}
    bbplain "[ssdk]make ${SD_SSDK_SEC}"
    make ${SD_SSDK_SEC}
    
}

do_compile[nostamp] += "1"
do_deploy[nostamp] += "1"

inherit deploy
do_deploy() {
    cd ${TOPDIR}/../source/ssdk
    install -m 0644 ${TOPDIR}/../source/ssdk/out/build-${SD_SSDK_SAF}/ssdk.bin ${DEPLOYDIR}/safety.bin
    install -m 0644 ${TOPDIR}/../source/ssdk/out/build-${SD_SSDK_SEC}/ssdk.bin ${DEPLOYDIR}/ssystem.bin
}

addtask do_deploy after do_compile
do_buildclean() {
    bbplain "make ${SD_SSDK_SEC} finish"
}

