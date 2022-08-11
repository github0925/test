SUMMARY = "Semidrive GStreamer test module"
DESCRIPTION = "For VPU GStreamer test"

inherit externalsrc

EXTERNALSRC = "${WORKDIR}/gstreamertest-${PV}"
EXTERNALSRC_BUILD = "${TOPDIR}/../source/vendor/vpu/gstreamer_test"

INC = "${WORKDIR}/recipe-sysroot/usr/include/"

LICENSE = "Apache-2.0"

TARGET_CC_ARCH += "${LDFLAGS}"

RDEPENDS_${PN} += "gtest gstreamer1.0 jsoncpp"
DEPENDS += "gtest gstreamer1.0 jsoncpp"

CXXFLAGS += "-I${INC}/gstreamer-1.0 -I${INC}/glib-2.0 -I${INC}/../lib/glib-2.0/include/"
EXTRA_OEMAKE += "CXXFLAGES='-L${INC}/../lib/' WORKDIR_CUR=${EXTERNALSRC}"

do_install () {
  install -d ${D}/usr/bin
  install -m 0755 ${EXTERNALSRC}/gstreamer_test_main ${D}/usr/bin/gstreamer_test_main
  install -m 0755 ${EXTERNALSRC}/gstreamer_test_module ${D}/usr/bin/gstreamer_test_module
}

do_clean () {
	cd ${EXTERNALSRC_BUILD}
	make clean
}

FILES_${PN} += "${bindir}/gstreamer_test_main ${bindir}/gstreamer_test_module"
FILES_${PN}-dbg += "${libdir}/.debug"
FILES_SOLIBSDEV = ""
INSANE_SKIP_${PN} = "dev-so"
