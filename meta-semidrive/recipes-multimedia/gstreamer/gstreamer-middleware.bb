SUMMARY = "Semidrive GStreamer middleware"
DESCRIPTION = "For VPU GStreamer middleware"

inherit externalsrc

EXTERNALSRC = "${WORKDIR}/gstreamer-middleware"
EXTERNALSRC_BUILD = "${TOPDIR}/../source/vendor/vpu/gstreamer_middleware"

INC = "${WORKDIR}/recipe-sysroot/usr/include/"

LICENSE = "Apache-2.0"

TARGET_CC_ARCH += "${LDFLAGS}"

RDEPENDS_${PN} += "gstreamer1.0 libdrm libhwconverter libminigbm"
DEPENDS += "gstreamer1.0 libdrm libhwconverter libminigbm"

CFLAGS += "-I${INC}/gstreamer-1.0 -I${INC}/glib-2.0 -I${INC}/../lib/glib-2.0/include/"
CFLAGS += "-I${INC}/libdrm"
EXTRA_OEMAKE += "CFLAGES='-L${INC}/../lib/' WORKDIR_CUR=${EXTERNALSRC}"

do_install () {
        install -d ${D}/usr/bin
        install -m 0755 ${EXTERNALSRC}/gstreamer_codec_test_main ${D}/usr/bin/gstreamer_codec_test_main

        install -d ${D}${libdir}
        install -m 0755 ${EXTERNALSRC}/libgstreamermiddleware.so ${D}${libdir}
}

FILES_${PN} += "${bindir}/* ${libdir}/*.so"
FILES_${PN}-dbg += "${libdir}/.debug"
FILES_SOLIBSDEV = ""
INSANE_SKIP_${PN} = "dev-so"
