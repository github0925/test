# makefile file
#
# Copyright (C) 2020 Semidrive Technology Co., Ltd.
#
# Description: compile libjpu.so
#
# Revision Histrory:
# -----------------
# 1.1, 10/12/2020  chentianming <tianming.chen@semidrive.com> create this file

SUMMARY = "Semidrive JPU lib"
DESCRIPTION = "Low-level lib based on chipsmedia JPU SDK"

inherit externalsrc

EXTERNALSRC = "${WORKDIR}/libjpu"
EXTERNALSRC_BUILD = "${TOPDIR}/../source/vendor/vpu/libjpu/jpuapi"

LICENSE = "Apache-2.0"
#LIC_FILES_CHKSUM = "file://readme;md5=3a46b04e50b52c451b19e511da3b2991"

RDEPENDS_${PN} = "libdrm"
DEPENDS = "libdrm"
CFLAGS += "-I${WORKDIR}/recipe-sysroot/usr/include/libdrm"

TARGET_CC_ARCH += "${LDFLAGS}"
EXTRA_OEMAKE += "WORKDIR_CUR=${EXTERNALSRC}"

do_install () {
	install -d ${D}/usr/lib/
	install -m 0755 ${EXTERNALSRC}/libjpu.so ${D}/usr/lib/
	install -m 0755 ${EXTERNALSRC}/libmiddlewarejpu.so ${D}/usr/lib/

	install -d ${D}/usr/include/
	install -m 0644 ${EXTERNALSRC_BUILD}/../jdi/include/jdi.h ${D}/usr/include/
	install -m 0644 ${EXTERNALSRC_BUILD}/include/jpuapi.h ${D}/usr/include/
	install -m 0644 ${EXTERNALSRC_BUILD}/include/jpuapifunc.h ${D}/usr/include/
	install -m 0644 ${EXTERNALSRC_BUILD}/include/jputypes.h ${D}/usr/include/
	install -m 0644 ${EXTERNALSRC_BUILD}/src/jpuconfig.h ${D}/usr/include/
	install -m 0644 ${EXTERNALSRC_BUILD}/middleware/jpu_middleware_api.h ${D}/usr/include/
	install -m 0644 ${EXTERNALSRC_BUILD}/middleware/drmutils.h ${D}/usr/include/
}

FILES_${PN} += "${libdir}/*.so ${includedir}/*.h"
FILES_${PN}-dbg += "${libdir}/.debug"
FILES_SOLIBSDEV = ""
INSANE_SKIP_${PN} = "dev-so"

RPROVIDES_${PN} += "libjpu"
