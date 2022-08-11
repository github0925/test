SUMMARY = "display examples: display test demo"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b30cbe0b980e98bfd9759b1e6ba3d107"

SRC_URI = "\
	file://LICENSE \
	file://Makefile \
	file://DrmDisplay.cpp \
	file://example01.cpp \
	file://example02.cpp \
	file://example03.cpp \
	file://example04.cpp \
	file://example05.cpp \
	file://DrmDisplay.h \
	file://debug.h \
	file://stb_image.h \
	"

S = "${WORKDIR}"

DEPENDS = "libdrm pvr-lib"


CFLAGS_prepend = "-I${STAGING_INCDIR}/drm"
CPPFLAGS_prepend = "-I${STAGING_INCDIR}/drm"

LDLIBS_append = "-ldrm"

FILES_${PN} = "\
	/usr/bin/example01 \
	/usr/bin/example02 \
	/usr/bin/example03 \
	/usr/bin/example04 \
	/usr/bin/example05 \
"

do_install () {
	install -d ${D}/usr/bin
	install -m 0755 example01 ${D}/usr/bin/example01
	install -m 0755 example02 ${D}/usr/bin/example02
	install -m 0755 example03 ${D}/usr/bin/example03
	install -m 0755 example04 ${D}/usr/bin/example04
	install -m 0755 example05 ${D}/usr/bin/example05
}
