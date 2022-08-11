LICENSE = "MIT"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SDRV_BACKEND = \
    "${@bb.utils.contains('DISTRO_FEATURES', 'wayland', 'wayland',\
        bb.utils.contains('DISTRO_FEATURES',     'x11',     'x11', \
                                                             'fb', d), d)}"
SRC_URI_append = " \
    file://qt5-${SDRV_BACKEND}.sh \
    file://001-Add-eglfs-for-multidisplay.patch \
"

PACKAGECONFIG_GL = "gles2"

QT_CONFIG_FLAGS_append = "\
    ${@bb.utils.contains('DISTRO_FEATURES', 'x11', '-no-eglfs', \
        bb.utils.contains('DISTRO_FEATURES', 'wayland', '-no-eglfs', \
            '-eglfs', d), d)}"

PACKAGECONFIG_WAYLAND ?= "${@bb.utils.contains('DISTRO_FEATURES', 'wayland', 'xkbcommon-evdev', '', d)}"
PACKAGECONFIG += "${PACKAGECONFIG_WAYLAND}"

DEPENDS = "${@bb.utils.contains('DISTRO_FEATURES', 'wayland', ' wayland ', '', d)}"

do_install_append () {
    if ls ${D}${libdir}/pkgconfig/Qt5*.pc >/dev/null 2>&1; then
        sed -i 's,-L${STAGING_DIR_HOST}/usr/lib,,' ${D}${libdir}/pkgconfig/Qt5*.pc
    fi
    install -d ${D}${sysconfdir}/profile.d/
    install -m 0755 ${WORKDIR}/qt5-${SDRV_BACKEND}.sh ${D}${sysconfdir}/profile.d/qt5.sh
}

FILES_${PN} += "${sysconfdir}/profile.d/qt5.sh"