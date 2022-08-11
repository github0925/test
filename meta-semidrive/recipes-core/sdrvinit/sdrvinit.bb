SUMMARY = "A bootup script to start semidrive services"
DESCRIPTION = "The script is a extenion for starting all semidrive services"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI = "file://COPYING \
           file://sdrv_service.sh \
"
inherit update-rc.d 

S = "${WORKDIR}"

INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "sdrv_service.sh"
INITSCRIPT_PARAMS_${PN} = "defaults 90"

#FILES_${PN} = "\
#	etc/init.d/sdrv_service.sh \
#"

#pkg_postinst_ontarget_${PN} () {
#
#}

do_install () {
	install -d ${D}${sysconfdir}/init.d
	install -m 0755 sdrv_service.sh ${D}${sysconfdir}/init.d
}
