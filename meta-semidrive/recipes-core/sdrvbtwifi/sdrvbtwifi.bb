SUMMARY = "A bootup script to start semidrive bt"
DESCRIPTION = "The script is a extenion for starting bt"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI = "file://COPYING \
           file://brcm_patchram_plus_latest \
           file://CYW89359B1_002.002.014.0153.0349.hcd \
           file://bluetooth.sh \
           file://wifi.sh \
           file://lib \
"
inherit update-rc.d

S = "${WORKDIR}"

INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "bluetooth.sh"
INITSCRIPT_PARAMS_${PN} = "defaults 90"
INSANE_SKIP_${PN} += "installed-vs-shipped"

#FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

#FILES_${PN} = "\
#	adbd \
#	sdrv_service.sh \
#	adb.sh \
#"

#pkg_postinst_ontarget_${PN} () {
#
#}

do_install () {
	install -d ${D}${sysconfdir}/init.d
	install -m 0755 bluetooth.sh ${D}${sysconfdir}/init.d
	install -m 0755 wifi.sh ${D}${sysconfdir}/init.d
	install -d ${D}${bindir}
	install -m 0755 brcm_patchram_plus_latest ${D}${bindir}/
	install -d ${D}${libdir}
	cp -rf CYW89359B1_002.002.014.0153.0349.hcd ${D}${libdir}/
	cp -rf lib ${D}/
}

FILES_${PN} += "lib lib/* ${libdir} ${libdir}/CYW89359B1_002.002.014.0153.0349.hcd"

