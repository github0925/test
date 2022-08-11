SUMMARY = "Gstreamer examples: gst-test demo"

LICENSE = "Gstreamer"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b30cbe0b980e98bfd9759b1e6ba3d107"

SRC_URI = "\
    file://LICENSE \
    file://Makefile \
    file://gst_test.c \
    "

S = "${WORKDIR}"

DEPENDS = "glib-2.0 gstreamer1.0"


CFLAGS_prepend = "-I${STAGING_INCDIR}/gstreamer-1.0/ -I${STAGING_INCDIR}/glib-2.0/"
CPPFLAGS_prepend = "-I${STAGING_INCDIR}/gstreamer-1.0/ -I${STAGING_INCDIR}/glib-2.0/"

LDLIBS_append = "-lpthread -lgstreamer-1.0 -lgstbase-1.0 -lgobject-2.0  pkg-config --cflags --libs gstreamer-1.0 glib-2.0"



FILES_${PN} = "\
    /usr/bin/gst_test \
"
do_configure () {
    cp ${STAGING_INCDIR}/../../../../../glib-2.0/1_2.54.3-r0/build/glib/glibconfig.h \
    ${STAGING_INCDIR}/glib-2.0/
}

do_install () {
    install -d ${D}/usr/bin
    install -m 0755 gst_test ${D}/usr/bin/gst_test
}
