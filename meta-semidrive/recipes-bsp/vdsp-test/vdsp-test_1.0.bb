SUMMARY = "vdsp examples: vdsp test demo"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b30cbe0b980e98bfd9759b1e6ba3d107"

SRC_URI = "\
	file://LICENSE \
	file://Makefile \
	file://vdsptest.cpp \
	file://csitest.h \
	file://xrp-linux-native/include/xrp_api.h \
	file://xrp-linux-native/include/xrp_kernel_defs.h \
	file://xrp-linux-native/include/xrp_xnnc_ns.h \
	file://xrp-linux-native/xrp_linux_native.c \
	file://g2d/g2dapi.c \
	file://g2d/include/g2dapi.h \
	file://g2d/include/sdrv_g2d_cfg.h \
	file://xtensa.elf \
	"

S = "${WORKDIR}"

DEPENDS = "libdrm"


CFLAGS_prepend = "-I${STAGING_INCDIR}/drm"
CXXFLAGS_prepend = "-I${STAGING_INCDIR}/drm"
CPPFLAGS_prepend = "-I${STAGING_INCDIR}/drm"

LDLIBS_append = "-ldrm"



FILES_${PN} = "\
	/usr/bin/vdsp-test\
	/lib/firmware/xtensa.elf\
"
INSANE_SKIP_${PN} = "arch"

do_install () {
	FIRMWARE_INSTALL_DIR=${D}${nonarch_base_libdir}/firmware

	install -d ${D}/usr/bin
	install -m 0755 vdsp-test ${D}/usr/bin/vdsp-test

	install -d -m 0755 ${D}/lib/firmware
	install -m 0644 xtensa.elf ${D}/lib/firmware/xtensa.elf

}
