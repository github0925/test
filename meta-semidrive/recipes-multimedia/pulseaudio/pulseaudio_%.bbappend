
# Enable allow-autospawn-for-root as default
PACKAGECONFIG_append = " autospawn-for-root"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI_append = " \
    file://pa-semidrive.sh \
	file://daemon.conf file://default.pa \
"

do_install_append () {
    install -d ${D}${sysconfdir}/profile.d/
	install -m 0644 ${WORKDIR}/daemon.conf ${D}${sysconfdir}/pulse/daemon.conf
    install -m 0644 ${WORKDIR}/default.pa ${D}${sysconfdir}/pulse/default.pa
    install -m 0755 ${WORKDIR}/pa-semidrive.sh ${D}${sysconfdir}/profile.d/pa-semidrive.sh
}

FILES_${PN} += "${sysconfdir}/profile.d/pa-semidrive.sh"
