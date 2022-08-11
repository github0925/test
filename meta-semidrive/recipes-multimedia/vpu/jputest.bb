# makefile file
#
# Copyright (C) 2020 Semidrive Technology Co., Ltd.
#
# Description:
#
# Revision Histrory:
# -----------------
# 1.1, 20/12/2020  chentianming <tianming.chen@semidrive.com> create this file

inherit externalsrc

EXTERNALSRC = "${WORKDIR}/jputest"
EXTERNALSRC_BUILD = "${TOPDIR}/../source/vendor/vpu/libjpu/test/"

LICENSE = "Apache-2.0"
#LIC_FILES_CHKSUM = "file://readme;md5=3a46b04e50b52c451b19e511da3b2991"

TARGET_CC_ARCH += "${LDFLAGS}"

FILES_${PN} += "${bindir}/jputest ${sysconfdir}"
FILES_${PN} += "${bindir}/multitest ${sysconfdir}"
FILES_${PN} += "${bindir}/jpumiddlewaretest ${sysconfdir}"

CFLAGS += "-I${WORKDIR}/recipe-sysroot/usr/include/libdrm"
EXTRA_OEMAKE += "WORKDIR_CUR=${EXTERNALSRC}"

RDEPENDS_${PN} = "libjpu libdrm"
DEPENDS = "libjpu libdrm"

do_install () {
	install -d ${D}/usr/bin

	install -m 0755 ${EXTERNALSRC}/jputest ${D}/usr/bin/jputest
	install -m 0755 ${EXTERNALSRC}/multitest ${D}/usr/bin/multitest
	install -m 0755 ${EXTERNALSRC}/jpumiddlewaretest ${D}/usr/bin/jpumiddlewaretest
}

RPROVIDES_${PN} += "jputest"
RPROVIDES_${PN} += "multitest"
RPROVIDES_${PN} += "jpumiddlewaretest"

