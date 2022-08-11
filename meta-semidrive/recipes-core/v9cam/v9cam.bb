SUMMARY = "v9cam"
DESCRIPTION = "the script for starting v9cam"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI = "file://COPYING \
           file://v9cam.sh \
"
inherit update-rc.d

S = "${WORKDIR}"

INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "v9cam.sh"
INITSCRIPT_PARAMS_${PN} = "defaults 90"
INSANE_SKIP_${PN} += "installed-vs-shippedi"
#FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

do_install () {
	install -d ${D}${sysconfdir}/init.d
	install -m 0755 v9cam.sh ${D}${sysconfdir}/init.d

}

#FILES_${PN} += "${libdir} ${libdir}/Fonts/*"


