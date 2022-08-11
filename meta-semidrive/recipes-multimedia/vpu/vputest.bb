SUMMARY = "Semidrive VPU Test Program"
DESCRIPTION = "For VPU Test"

inherit externalsrc

EXTERNALSRC = "${TOPDIR}/../source/vendor/vpu"
EXTERNALSRC_BUILD = "${EXTERNALSRC}/libvpu/test/sample"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://readme;md5=3a46b04e50b52c451b19e511da3b2991"

TARGET_CC_ARCH += "${LDFLAGS}"

FILES_${PN} += "${bindir}/vputest ${sysconfdir}"
RDEPENDS_${PN} = "libvpu ffmpeg"

DEPENDS = "libvpu ffmpeg"
EXTRA_OEMAKE += "WORKDIR_CUR=${WORKDIR}"

do_install () {
	install -d ${D}/usr/bin
	install -m 0755 vputest ${D}/usr/bin/vputest
}

do_clean () {
	cd ${EXTERNALSRC_BUILD}
	make clean
}

RPROVIDES_${PN} += "vputest"
