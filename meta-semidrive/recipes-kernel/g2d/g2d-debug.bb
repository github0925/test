SUMMARY = "build external G2D Linux kernel module"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${THISDIR}/files/COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"

SRC_URI = "file://sdrv-g2d.ko\
            file://COPYING"

S = "${WORKDIR}"
do_compile(){
	echo compiling
}

inherit linux-kernel-base kernel-module-split

do_install[depends] += "virtual/kernel:do_shared_workdir"
do_install[nostamp] += "1"

B = "${STAGING_KERNEL_BUILDDIR}"
KERNEL_VERSION = "${@get_kernelversion_headers('${B}')}"

# add local makefile patch for yocto
# the original Makefile is for android distribution
do_install () {
    MODULE_DIR=${D}${nonarch_base_libdir}/modules/${KERNEL_VERSION}/kernel/drivers/g2d
    install -d ${MODULE_DIR}
    install -m 755 ${S}/*.ko ${MODULE_DIR}
}

# The inherit of module.bb class will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.
RPROVIDES_${PN} += "kernel-module-g2d"

# let the build system extends the FILESPATH file search path
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

FILES_${PN} += "\
                /lib \
                /lib/modules/${KERNEL_VERSION}/kernel/drivers/g2d/sdrv-g2d.ko \
                "

KERNEL_MODULE_AUTOLOAD = "sdrv-g2d"

# kernel-module-kunlun-g2d* rdepends on kernel-4.14.61, ignore the warning
INSANE_SKIP_${PN} += "build-deps"
