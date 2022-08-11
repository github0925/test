SUMMARY = "camera examples: csi test demo"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b30cbe0b980e98bfd9759b1e6ba3d107"

SRC_URI = "\
	file://LICENSE \
	file://Makefile \
	file://csitest.cpp \
	file://csitest_gl.cpp \
	file://csitest.h \
	file://shaders \
	"

S = "${WORKDIR}"

DEPENDS = "libdrm pvr-lib"


CFLAGS_prepend = "-I${STAGING_INCDIR}/drm"
CPPFLAGS_prepend = "-I${STAGING_INCDIR}/drm"

LDLIBS_append = "-ldrm"



FILES_${PN} = "\
	/usr/bin/csi-test\
	/usr/bin/csi-test-gl\
	/usr/bin/shaders/*\
"


do_install () {
	SHADERS_INSTALL_DIR=${D}/usr/bin/shaders

	install -d ${D}/usr/bin
	install -m 0755 csi-test ${D}/usr/bin/csi-test
	install -m 0755 csi-test-gl ${D}/usr/bin/csi-test-gl
	mkdir -p ${SHADERS_INSTALL_DIR}
	install -m 644 ${S}/shaders/* ${SHADERS_INSTALL_DIR}
}
