SUMMARY = "A bootup script to start semidrive services"
DESCRIPTION = "The script is a extenion for starting all semidrive services"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI = "file://COPYING \
           file://adbd \
           file://adb.sh \
"
inherit update-rc.d

S = "${WORKDIR}"

INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "adb.sh"
INITSCRIPT_PARAMS_${PN} = "defaults 90"
INSANE_SKIP_${PN} += "installed-vs-shipped"

#FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

FILES_${PN} += "\
    /system/bin \
    /data \
    /config \
"
#pkg_postinst_ontarget_${PN} () {
#
#}

do_install () {
	install -d ${D}${sysconfdir}/init.d
	install -m 0755 adb.sh ${D}${sysconfdir}/init.d
	install -d ${D}${bindir}
	install -m 0755 adbd ${D}${bindir}/

    install -d ${D}/system/bin
    install -d -m 777 ${D}/data
    install -d  ${D}/config

    ln -sf /bin/sh ${D}/system/bin/sh
}


