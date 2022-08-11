SUMMARY = "sdcam"
DESCRIPTION = "the script for starting sdcam"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI = "file://COPYING \
           file://sdcam.sh \
"
inherit update-rc.d

S = "${WORKDIR}"

INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "sdcam.sh"
INITSCRIPT_PARAMS_${PN} = "defaults 90"
INSANE_SKIP_${PN} += "installed-vs-shippedi"
#FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

do_install () {
	install -d ${D}${sysconfdir}/init.d
	install -m 0755 sdcam.sh ${D}${sysconfdir}/init.d

}

#FILES_${PN} += "${libdir} ${libdir}/Fonts/*"


