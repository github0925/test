SUMMARY = "vdsp avm: vdsp avm demo"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b30cbe0b980e98bfd9759b1e6ba3d107"

SRC_URI = "\
	file://LICENSE \
	file://Makefile \
	file://vdspavm.cpp \
	file://csitest.h \
	file://xrp-linux-native/include/xrp_api.h \
	file://xrp-linux-native/include/xrp_kernel_defs.h \
	file://xrp-linux-native/include/xrp_xnnc_ns.h \
	file://xrp-linux-native/xrp_linux_native.c \
	file://avm.elf \
	file://maptable.bin \
	file://avm.conf \
	"

S = "${WORKDIR}"

DEPENDS = "libdrm"


CFLAGS_prepend = "-I${STAGING_INCDIR}/drm"
CXXFLAGS_prepend = "-I${STAGING_INCDIR}/drm"
CPPFLAGS_prepend = "-I${STAGING_INCDIR}/drm"

LDLIBS_append = "-ldrm"



FILES_${PN} = "\
	/usr/bin/vdsp-avm\
	/lib/firmware/avm.elf\
	/lib/firmware/maptable.bin\
	/lib/firmware/avm.conf\
"
INSANE_SKIP_${PN} = "arch"

do_install () {
	FIRMWARE_INSTALL_DIR=${D}${nonarch_base_libdir}/firmware

	install -d ${D}/usr/bin
	install -m 0755 vdsp-avm ${D}/usr/bin/vdsp-avm

	install -d -m 0755 ${D}/lib/firmware
	install -m 0644 avm.elf ${D}/lib/firmware/avm.elf
	install -m 0644 maptable.bin ${D}/lib/firmware/maptable.bin
	install -m 0644 avm.conf ${D}/lib/firmware/avm.conf

}
