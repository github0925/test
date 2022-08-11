SUMMARY = "controlpanel"
DESCRIPTION = "the script for starting controlpanel"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI = "file://COPYING \
           file://aircon \
           file://Fonts \
           file://controlpanel.sh \
"
inherit update-rc.d

S = "${WORKDIR}"

INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "controlpanel.sh"
INITSCRIPT_PARAMS_${PN} = "defaults 90"
INSANE_SKIP_${PN} += "installed-vs-shippedi ldflags"
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
DEPENDS = "pvr-lib messagecenter"

do_install () {
	install -d ${D}${sysconfdir}/init.d
	install -m 0755 controlpanel.sh ${D}${sysconfdir}/init.d
	install -d ${D}${bindir}
	install -m 0755 aircon ${D}${bindir}/
	install -d ${D}${libdir}/
	install -d ${D}${libdir}/Fonts/
	cp -rf Fonts/* ${D}${libdir}/Fonts/

}

FILES_${PN} += "${libdir} ${libdir}/Fonts/*"


