SUMMARY = "Bellagio OpenMAX Integration Layer (IL)"
DESCRIPTION = "Bellagio is an opensource implementation of the Khronos OpenMAX \
				Integration Layer API to access multimedia components."
HOMEPAGE = "http://omxil.sourceforge.net/"

inherit externalsrc

EXTERNALSRC = "${TOPDIR}/../source/vendor/vpu"
EXTERNALSRC_BUILD = "${EXTERNALSRC}/openmax/libomxil-bellagio-0.9.3"
INC = "${WORKDIR}/recipe-sysroot/usr/include/"

LICENSE = "LGPLv2.1+"
LICENSE_FLAGS = "commercial"
LIC_FILES_CHKSUM = "file://openmax/libomxil-bellagio-0.9.3/COPYING;md5=89a0a92a13006294ec613ecc2fe53691 \
                    file://openmax/libomxil-bellagio-0.9.3/src/omxcore.h;beginline=1;endline=27;md5=806b1e5566c06486fe8e42b461e03a90"

TARGET_CC_ARCH += "${LDFLAGS}"
RDEPENDS_${PN} += "gstreamer1.0"
DEPENDS += "gstreamer1.0"

CFLAGS += "-DGSTREAMER_LOG"
CFLAGS += "-I${INC}/gstreamer-1.0 -I${INC}/glib-2.0 -I${INC}/../lib/glib-2.0/include/"
EXTRA_OEMAKE += "CFLAGES='-L${INC}/../lib/ -lgstreamer-1.0' WORKDIR_CUR=${WORKDIR}"

do_install () {
	install -d ${D}${libdir}
	install -m 0755 libomxil-bellagio.so ${D}${libdir}
	ln -srf ${D}${libdir}/libomxil-bellagio.so ${D}${libdir}/libomxil-bellagio.so.0

	install -d ${D}${sysconfdir}/xdg
	install -m 0644 ${S}/openmax/libomxil-bellagio-0.9.3/.omxregister_gst ${D}${sysconfdir}/xdg/.omxregister
}

do_clean () {
	cd ${EXTERNALSRC_BUILD}
	make clean
}

FILES_${PN} += "${libdir}/*.so ${sysconfdir}/xdg/.omxregister"
FILES_${PN}-dbg += "${libdir}/.debug"
FILES_SOLIBSDEV = ""
INSANE_SKIP_${PN} = "dev-so"

RPROVIDES_${PN} += "libomxil"
