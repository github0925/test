SUMMARY = "build external G2D Linux kernel module"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"

inherit module

SEMIDRIVE_BSP_URL = " \
    git://gerrit.semidrive.net:8081/tools"
BRANCH = "master"
SRCREV = "b23419e96dc2006c419fa2a45d96c923a325e0dd"

SRC_URI = "${SEMIDRIVE_BSP_URL};protocol=http;branch=${BRANCH} \
           file://Makefile \
           file://COPYING"

REPO_SRC_DIR = "${TOPDIR}/../source"

S = "${WORKDIR}/git/display/external_drm/sdrv_g2d"

# add local makefile patch for yocto
# the original Makefile is for android distribution
do_patch () {
    cp -p ${THISDIR}/files/COPYING ${S}
    cp -p ${THISDIR}/files/Makefile ${S}
}

MAKE_TARGETS = "modules"

# suppose this task is done in linux kernel build
do_configure[depends] += "virtual/kernel:do_shared_workdir"

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.
##RPROVIDES_${PN} += "kernel-module-g2d"

# let the build system extends the FILESPATH file search path
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"