SUMMARY = "Semidrive VPU OMX lib"
DESCRIPTION = "OpenMax IL Component lib for VPU"

inherit externalsrc

EXTERNALSRC = "${TOPDIR}/../source/vendor/vpu"
EXTERNALSRC_BUILD = "${EXTERNALSRC}/openmax/omxil-vpu-component"
INC = "${WORKDIR}/recipe-sysroot/usr/include/"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://readme;md5=3a46b04e50b52c451b19e511da3b2991"

TARGET_CC_ARCH += "${LDFLAGS}"

RDEPENDS_${PN} += "libvpu libomxil gstreamer1.0"
DEPENDS += "libvpu libomxil gstreamer1.0"

CFLAGS += "-DGSTREAMER_LOG"
CFLAGS += "-I${INC}/gstreamer-1.0 -I${INC}/glib-2.0 -I${INC}/../lib/glib-2.0/include/"
EXTRA_OEMAKE += "CFLAGES='-L${INC}/../lib/ -lgstreamer-1.0' WORKDIR_CUR=${WORKDIR}"

do_install () {
	install -d ${D}${libdir}
	install -m 0755 libomxvpu.so ${D}${libdir}
}

do_clean () {
	cd ${EXTERNALSRC_BUILD}
	make clean
}

FILES_${PN} += "${libdir}/*.so ${sysconfdir}"
FILES_${PN}-dbg += "${libdir}/.debug"
FILES_SOLIBSDEV = ""
INSANE_SKIP_${PN} = "dev-so"

RPROVIDES_${PN} += "libomxvpu"
