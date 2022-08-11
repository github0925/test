SUMMARY = "Semidrive VPU lib"
DESCRIPTION = "Low-level lib based on chipsmedia VPU SDK"

inherit externalsrc

EXTERNALSRC = "${TOPDIR}/../source/vendor/vpu"
EXTERNALSRC_BUILD = "${EXTERNALSRC}/libvpu"
INC = "${WORKDIR}/recipe-sysroot/usr/include/"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://readme;md5=3a46b04e50b52c451b19e511da3b2991"

TARGET_CC_ARCH += "${LDFLAGS}"

RDEPENDS_${PN} += "gstreamer1.0"
DEPENDS += "gstreamer1.0"

CFLAGS += "-DGSTREAMER_LOG"
CFLAGS += "-I${INC}/gstreamer-1.0 -I${INC}/glib-2.0 -I${INC}/../lib/glib-2.0/include/"
EXTRA_OEMAKE += "CFLAGES='-L${INC}/../lib/ -lgstreamer-1.0' WORKDIR_CUR=${WORKDIR}"

do_install () {
	install -d ${D}${libdir}
	install -m 0755 libvpu.so ${D}${libdir}

	FIRMWARE_INSTALL_DIR=${D}${nonarch_base_libdir}/firmware

	install -d -m 0755 ${FIRMWARE_INSTALL_DIR}
	install -m 0644 fw/coda980.out ${FIRMWARE_INSTALL_DIR}
	install -m 0644 fw/pissarro.bin ${FIRMWARE_INSTALL_DIR}

	install -d ${D}/usr/bin
	ln -srf ${FIRMWARE_INSTALL_DIR}/coda980.out ${D}/usr/bin/coda980.out
	ln -srf ${FIRMWARE_INSTALL_DIR}/pissarro.bin ${D}/usr/bin/pissarro.bin
}

do_clean () {
	cd ${EXTERNALSRC_BUILD}
	make clean
}

FILES_${PN} += "${libdir}/*.so ${nonarch_base_libdir}/firmware/*"
FILES_${PN}-dbg += "${libdir}/.debug"
FILES_SOLIBSDEV = ""
INSANE_SKIP_${PN} = "dev-so"

RPROVIDES_${PN} += "libvpu"
