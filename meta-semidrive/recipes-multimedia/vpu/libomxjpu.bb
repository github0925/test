SUMMARY = "Semidrive JPU OMX lib"
DESCRIPTION = "OpenMax IL Component lib for JPU"

inherit externalsrc

EXTERNALSRC = "${WORKDIR}/libomxjpu"
EXTERNALSRC_BUILD = "${TOPDIR}/../source/vendor/vpu/openmax/omxil-jpu-component"
INC = "${WORKDIR}/recipe-sysroot/usr/include/"

TARGET_CC_ARCH += "${LDFLAGS}"

LICENSE = "Apache-2.0"

RDEPENDS_${PN} += "libjpu libomxil gstreamer1.0"
DEPENDS += "libjpu libomxil gstreamer1.0"

CFLAGS += "-DGSTREAMER_LOG"
CFLAGS += "-I${INC}/gstreamer-1.0 -I${INC}/glib-2.0 -I${INC}/../lib/glib-2.0/include/"
EXTRA_OEMAKE += "CFLAGES='-L${INC}/../lib/ -lgstreamer-1.0' WORKDIR_CUR=${EXTERNALSRC}"

do_install () {
	install -d ${D}${libdir}
	install -m 0755 ${EXTERNALSRC}/libomxjpu.so ${D}${libdir}
}

do_clean () {
	cd ${EXTERNALSRC_BUILD}
	make clean
}

FILES_${PN} += "${libdir}/*.so ${sysconfdir}"
FILES_${PN}-dbg += "${libdir}/.debug"
FILES_SOLIBSDEV = ""
INSANE_SKIP_${PN} = "dev-so"

RPROVIDES_${PN} += "libomxjpu"
