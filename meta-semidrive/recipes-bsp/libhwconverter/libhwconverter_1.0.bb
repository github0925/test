SUMMARY = "2d hardware convert library"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b30cbe0b980e98bfd9759b1e6ba3d107"

SRC_URI = "\
    file://LICENSE \
    file://Makefile \
    file://display/ \
    file://allocator/ \
    file://HwConverter.cpp \
    file://buildConverter.cpp \
    file://G2dConverter.cpp \
    file://tests/ \
    file://include/ \
    "

S = "${WORKDIR}"
DEPENDS = "libdrm libminigbm"
LDLIBS_append = "-L${WORKDIR} -ldrm"

SDKTARGETSYSROOT="${STAGING_DIR_TARGET}"

CFLAGS_prepend = "-I${STAGING_INCDIR}/drm -DSDKTARGETSYSROOT =${STAGING_DIR_TARGET}"
CPPFLAGS_prepend = "-I${STAGING_INCDIR}/drm"

FILES_${PN} = "\
    /usr/lib/libhwconverter.so \
    /usr/lib/libdrmdisplay.so \
    /usr/bin/test_hwconverter \
    "

FILES_SOLIBSDEV = ""

do_configure_prepend() {
    builddir=`readlink -f ${WORKDIR}/../../../../.. | awk -F "/" '{ print $NF }'`
    bbplain "current dir: `pwd`"
}

do_compile() {
    WORKDIR=${S} SDKTARGETSYSROOT=${STAGING_DIR_TARGET} STAGING_KERNEL_DIR=${STAGING_KERNEL_DIR} make
}

do_install () {
    install -d ${D}/usr/lib
#    install -m 0755 libhwconverter.so ${D}/usr/lib/libhwconverter.so
#    install -m 0755 libdrmdisplay.so ${D}/usr/lib/libdrmdisplay.so
    install -m 0755 lib*.so ${D}/usr/lib/
    install -d ${D}/${includedir}
    #cp -r ${S}/include/* ${D}/${includedir}
    install  ${S}/include/* ${D}/${includedir}
}

INSANE_SKIP_${PN} = "ldflags"

INSANE_SKIP_${PN}-dev = "ldflags"


