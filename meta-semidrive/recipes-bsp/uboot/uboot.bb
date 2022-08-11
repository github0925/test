#
# Recipe for uboot
SUMMARY = "uboot "
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

inherit externalsrc

EXTERNALSRC = "${TOPDIR}/../source/u-boot"
EXTERNALSRC_BUILD = "${EXTERNALSRC}"

PATH_append = \
    ":/tool/gcc_linaro/gcc-arm-none-eabi-7.3.1/bin:/tool/gcc_linaro/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-elf/bin:/tool/gcc_linaro/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin:/usr/sbin:/usr/bin:"
CFLAGS=""

def get_sd_bl_type(d):
    origenv = d.getVar("BB_ORIGENV", False)
    bl_type = origenv.getVar("YOCTO_BL_TYPE", False)
    return bl_type
yocto_bl_type = "${@get_sd_bl_type(d)}"
MACHAINE_BOOTLOADER_TYPE ?= "lk"

EXTRA_OEMAKE = "${MACHINE_UBOOT_MAKE_EXTRA_OPTION}"

do_configure() {
    echo "skip this step"
}

do_compile() {

    if [ "${MACHAINE_BOOTLOADER_TYPE}" = "uboot" ] ;then
    (
        make ${EXTRA_OEMAKE} ${MACHINE_UBOOT_DEFCONF}
        make ${EXTRA_OEMAKE} distclean
        cd ${EXTERNALSRC_BUILD}

        cd -
        make ${EXTRA_OEMAKE} ${MACHINE_UBOOT_DEFCONF}
        make ${EXTRA_OEMAKE}
    )
   fi
    
}

do_compile[nostamp] += "1" 
do_deploy[nostamp] += "1"

inherit deploy
do_deploy() {
    if [ "${MACHAINE_BOOTLOADER_TYPE}" = "uboot" ] ;then
    (
        echo "do_deploy"
        install -m 0644 ./u-boot.bin ${DEPLOYDIR}/${MACHINE_UBOOT_BOOTLOADER}
    )
    fi
}

addtask do_deploy after do_compile
do_buildclean() {
    rm -rf build-*
}

##FILES_${PN} = "${bindir}/*"
