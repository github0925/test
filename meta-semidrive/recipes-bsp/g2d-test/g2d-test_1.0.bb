SUMMARY = "camera examples: g2d test demo"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b30cbe0b980e98bfd9759b1e6ba3d107"

SRC_URI = "\
	file://LICENSE \
	file://Makefile \
	file://g2dapi.c \
	file://g2dapi.h \
	file://main.cpp \
	file://drm_handle.c \
	file://drm_handle.h \
	file://g2d_test_utils.h \
	"

S = "${WORKDIR}"

DEPENDS = "libdrm pvr-lib"


CFLAGS_prepend = "-I${STAGING_INCDIR}/drm"
CPPFLAGS_prepend = "-I${STAGING_INCDIR}/drm"

LDLIBS_append = "-ldrm"



FILES_${PN} = "\
	/usr/bin/g2d_test \
"


do_install () {
	install -d ${D}/usr/bin
	install -m 0755 g2d_test ${D}/usr/bin/g2d_test
}
