SUMMARY = "Extra machine specific configuration files"
HOMEPAGE = "https://wiki.gentoo.org/wiki/Eudev"
DESCRIPTION = "Extra machine specific configuration files for udev, specifically blacklist information."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI = " \
       file://automount.rules \
       file://mount.sh \
       file://mount.blacklist \
       file://autonet.rules \
       file://network.sh \
       file://localextra.rules \
       file://by-name.rules \
       file://by-name.sh \
"

S = "${WORKDIR}"


do_install() {
    install -d ${D}${sysconfdir}/udev/rules.d

    install -m 0644 ${WORKDIR}/automount.rules     ${D}${sysconfdir}/udev/rules.d/automount.rules
    install -m 0644 ${WORKDIR}/autonet.rules       ${D}${sysconfdir}/udev/rules.d/autonet.rules
    install -m 0644 ${WORKDIR}/localextra.rules    ${D}${sysconfdir}/udev/rules.d/localextra.rules
    install -m 0644 ${WORKDIR}/by-name.rules       ${D}${sysconfdir}/udev/rules.d/by-name.rules

    install -d ${D}${sysconfdir}/udev/mount.blacklist.d
    install -m 0644 ${WORKDIR}/mount.blacklist     ${D}${sysconfdir}/udev/

    install -d ${D}${sysconfdir}/udev/scripts/

    install -m 0755 ${WORKDIR}/mount.sh ${D}${sysconfdir}/udev/scripts/mount.sh
    install -m 0755 ${WORKDIR}/network.sh ${D}${sysconfdir}/udev/scripts
    install -m 0755 ${WORKDIR}/by-name.sh ${D}${sysconfdir}/udev/scripts/by-name.sh
}

FILES_${PN} = "${sysconfdir}/udev"
RDEPENDS_${PN} = "udev"
CONFFILES_${PN} = "${sysconfdir}/udev/mount.blacklist"

# to replace udev-extra-rules from meta-oe
RPROVIDES_${PN} = "udev-extra-rules"
RREPLACES_${PN} = "udev-extra-rules"
RCONFLICTS_${PN} = "udev-extra-rules"
