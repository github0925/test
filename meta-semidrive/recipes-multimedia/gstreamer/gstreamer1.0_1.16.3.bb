SUMMARY = "Install prebuilt gstreamer libraries"
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

S = "${WORKDIR}"

PV = "1.16.3"

SRC_URI = "\
	file://etc \
	file://usr \
"

DEPENDS = "glib-2.0 glib-2.0-native wayland"

## prebuilt library don't need following steps
do_configure[noexec] = "1"
do_compile[noexec] = "1"
do_package_qa[noexec] = "1"
do_install[nostamp] += "1"

do_install () {
    install -d ${D}${libdir}
    cp -rdf ${S}/usr/lib/* ${D}${libdir}

    # Install headers and pkgconfig
    install -d ${D}${includedir}
    cp -rdf ${S}/usr/include/* ${D}${includedir}

    install -d ${D}/etc/xdg
    cp -rf ${S}/etc/xdg/gstomx.conf ${D}/etc/xdg/

    install -d ${D}${bindir}
    cp -rf ${S}/usr/bin/* ${D}${bindir}

    install -d ${D}${libexecdir}
    cp -rdf ${S}/usr/libexec/* ${D}${libexecdir}
}

# let the build system extends the FILESPATH file search path
FILESEXTRAPATHS_prepend := "${THISDIR}/prebuilts:"

FILES_${PN} += "${libdir}/*.so ${libdir}/gstreamer-1.0/*.so ${libdir}/lib*${SOLIBS} ${sysconfdir}/xdg/gstomx.conf ${bindir}/*"

INSANE_SKIP_${PN} += "installed-vs-shipped"
