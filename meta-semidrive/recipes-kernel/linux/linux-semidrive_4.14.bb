DESCRIPTION = "Linux kernel for the Semidrive SoC based board"

require recipes-kernel/linux/linux-yocto.inc

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}/:"
COMPATIBLE_MACHINE = "x9high|g9|v9f|d9"

SEMIDRIVE_BSP_URL = " \
    git://gerrit.semidrive.net:8081/android/kernel/common"
BRANCH = "X9_Android_Intergration_B"
SRCREV = "d5623fc4c1453a0c9b48697a685fdfd961614bd5"

SRC_URI = "${SEMIDRIVE_BSP_URL};protocol=http;nocheckout=1;branch=${BRANCH}"

LINUX_VERSION ?= "4.14.61"
PV = "${LINUX_VERSION}+git${SRCPV}"
PR = "r1"

do_patch_gerrit () {
}

addtask do_patch_gerrit after do_patch before do_configure
