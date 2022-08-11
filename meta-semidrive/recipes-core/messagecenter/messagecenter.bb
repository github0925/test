SUMMARY = "messagecenter"
DESCRIPTION = "the script for starting messagecenter"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI = "file://COPYING \
           file://messagecenter \
           file://haveged \
           file://libhavege.so.1.1.0 \
           file://messagecenter.sh \
"
inherit update-rc.d

S = "${WORKDIR}"

INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "messagecenter.sh"
INITSCRIPT_PARAMS_${PN} = "defaults 80"
INSANE_SKIP_${PN} += "installed-vs-shippedi ldflags"
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

do_install () {
	install -d ${D}${sysconfdir}/init.d
	install -m 0755 messagecenter.sh ${D}${sysconfdir}/init.d
	install -d ${D}${bindir}
	install -m 0755 messagecenter ${D}${bindir}/
	install -d ${D}${libdir}/
	cp -prf --no-preserve=ownership libhavege.so.1.1.0 ${D}${libdir}/libhavege.so.1
	cp -prf --no-preserve=ownership haveged ${D}${bindir}/
	chmod 755 ${D}${bindir}/haveged

}
