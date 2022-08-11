SUMMARY = "sdhud"
DESCRIPTION = "the script for starting sdhud"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;md5=838c366f69b72c5df05c96dff79b35f2"

SRC_URI = "file://COPYING \
           file://sdhud \
           file://sdhud.sh \
"
inherit update-rc.d

S = "${WORKDIR}"

INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "sdhud.sh"
INITSCRIPT_PARAMS_${PN} = "defaults 90"
INSANE_SKIP_${PN} += "already-stripped installed-vs-shipped ldflags"
#FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
RDEPENDS_${PN} += " qtbase qtgraphicaleffects qtquickcontrols qtquickcontrols2 "
DEPENDS = "pvr-lib qtbase qtserialport qtdeclarative qtquickcontrols qtquickcontrols2 qtgraphicaleffects "

do_install () {
    install -d ${D}${sysconfdir}/init.d
    install -m 0755 sdhud.sh ${D}${sysconfdir}/init.d
    install -d ${D}${bindir}
    install -m 0755 sdhud ${D}${bindir}/
    install -d ${D}${libdir}/
}


