LICENSE = "GPLv2"
DESCRIPTION = "Linux 5.4 kernel for the Semidrive SoC based board"
KERNEL_CONFIG_COMMAND ?= "oe_runmake_call -C ${S} CC="${KERNEL_CC}" O=${B} olddefconfig"

inherit kernel
require recipes-kernel/linux/linux-yocto.inc

# Override kernel license checksum
LIC_FILES_CHKSUM = "file://COPYING;md5=bbea815ee2795b2f4230826c0c6b8814"
FILES_${KERNEL_PACKAGE_NAME}-base += "${nonarch_base_libdir}/modules/${KERNEL_VERSION}/modules.builtin.modinfo"

# Allows to avoid fetching, unpacking and patching, since our code is already cloned by repo
inherit externalsrc

COMPATIBLE_MACHINE = "g9|v9f"

REPO_SRC_DIR = "${TOPDIR}/../source"
LINUX_VERSION ?= "5.4.6"
PV = "${LINUX_VERSION}"
PR = "r1"

# Don't use Yocto kernel configuration system, we instead simply override do_configure
# to copy our defconfig in the build directory just before building.
# I agree this is very ad hoc, but maybe it's good enough for our development environment
#do_configure() {
##  cp "${REPO_TOP_DIR}/source/linux/arch/arm64/configs/x9_evb_defconfig" "${B}/.config"
#}

EXTERNALSRC_pn-linux-semidrive-dev = "${REPO_SRC_DIR}/linux5.4"
EXTERNALSRC_BUILD_pn-linux-semidrive-dev = "${B}"
LINUX_VERSION_EXTENSION = "-smdv-${LINUX_KERNEL_TYPE}"

#S = "${REPO_SRC_DIR}/linux"
#B = "${REPO_SRC_DIR}/linux"

# This is required for kernel to do the build out-of-tree.
# If this is not set, most of the kernel make targets won't work properly
# as they'll be executed in the sources
export KBUILD_OUTPUT="${B}"

# The previous line should not be necessary when those 2 are added
# but it doesn't work..
KBUILD_OUTPUT = "${B}"
OE_TERMINAL_EXPORTS += "KBUILD_OUTPUT"

# kernel-source directory is required to build external modules like drm-dev
#
do_shared_workdir_append () {
    rm -rf ${STAGING_KERNEL_DIR}
    ln -sf ${S} ${STAGING_KERNEL_DIR}
}

do_deploy() {
  kernel_do_deploy
##  install ${B}/vmlinux ${DEPLOYDIR}/vmlinux
}

addtask do_shared_workdir after do_compile before do_install

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}/:"
