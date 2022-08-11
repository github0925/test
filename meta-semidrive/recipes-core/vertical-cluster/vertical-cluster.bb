SUMMARY = "vertical-cluster"
DESCRIPTION = "the script for starting cluster"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI = "file://COPYING \
           file://vertical-cluster \
           file://Fonts \
           file://vertical-cluster.sh \
"
inherit update-rc.d

S = "${WORKDIR}"

INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "vertical-cluster.sh"
INITSCRIPT_PARAMS_${PN} = "defaults 90"
DEPENDS = "pvr-lib"
INSANE_SKIP_${PN} += "already-stripped installed-vs-shippedi ldflags"
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

do_install () {
	install -d ${D}${sysconfdir}/init.d
	install -m 0755 vertical-cluster.sh ${D}${sysconfdir}/init.d
	install -d ${D}${bindir}
	install -m 0755 vertical-cluster ${D}${bindir}/
	install -d ${D}${libdir}/
	install -d ${D}${libdir}/Fonts/
	cp -rf Fonts/* ${D}${libdir}/Fonts/

}

FILES_${PN} += "${libdir} ${libdir}/Fonts/*"


