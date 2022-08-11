SUMMARY = "Install prebuilt Mesa library "
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

SRC_URI = "file://include \
           file://lib \
           file://share \
          "

S = "${WORKDIR}/"

PV = "20.0.1"
PE = "2"

## prebuilt library don't need following steps
do_configure[noexec] = "1"
do_compile[noexec] = "1"
do_install[nostamp] += "1"

do_install () {
    install -d ${D}${libdir}
    install -d ${D}${libdir}/dri
    cp -rdf ${S}/lib/* ${D}${libdir}

    # Install headers and pkgconfig
    install -d ${D}/${includedir}
    install -d ${D}${libdir}/pkgconfig
    cp -r ${S}/include/* ${D}/${includedir}
    cp -r ${S}/lib/pkgconfig/* ${D}/${libdir}/pkgconfig
}

# let the build system extends the FILESPATH file search path
FILESEXTRAPATHS_prepend := "${@bb.utils.contains('DISTRO_FEATURES', 'x11', '${TOPDIR}/../prebuilt/gpu/mesa_xorg:', '${TOPDIR}/../prebuilt/gpu/mesa:', d)}"

DEPENDS = "expat makedepend-native flex-native bison-native libxml2-native zlib chrpath-replacement-native"
DEPENDS += "${@bb.utils.contains('DISTRO_FEATURES', 'x11', 'libdrm libxcb libxshmfence libxxf86vm libxshmfence libxfixes libxdamage', '', d)}"

PROVIDES = " \
    ${@bb.utils.contains('PACKAGECONFIG', 'opengl', 'virtual/libgl', '', d)} \
    ${@bb.utils.contains('PACKAGECONFIG', 'gles', 'virtual/libgles1 virtual/libgles2', '', d)} \
    ${@bb.utils.contains('PACKAGECONFIG', 'egl', 'virtual/egl', '', d)} \
    ${@bb.utils.contains('PACKAGECONFIG', 'gbm', 'virtual/libgbm', '', d)} \
    virtual/mesa \
    "

ANY_OF_DISTRO_FEATURES = "opengl vulkan"

PLATFORMS ??= "${@bb.utils.filter('PACKAGECONFIG', 'x11 wayland', d)} \
               ${@bb.utils.contains('PACKAGECONFIG', 'gbm', 'drm', '', d)}"

PACKAGECONFIG ??= "${@bb.utils.filter('DISTRO_FEATURES', 'wayland vulkan', d)} \
                   ${@bb.utils.contains('DISTRO_FEATURES', 'opengl', 'opengl egl gles gbm dri', '', d)} \
                   ${@bb.utils.contains('DISTRO_FEATURES', 'x11 opengl', 'x11 dri3', '', d)} \
                   ${@bb.utils.contains('DISTRO_FEATURES', 'x11 vulkan', 'dri3', '', d)} \
		   "

# Remove the mesa dependency on mesa-dev, as mesa is empty
RDEPENDS_${PN}-dev = ""

# Add dependency so that GLES3 header don't need to be added manually
RDEPENDS_libgles2-mesa-dev += "libgles3-mesa-dev"

PACKAGES =+ "libegl-mesa libegl-mesa-dev \
             libosmesa libosmesa-dev \
             libgl-mesa libgl-mesa-dev \
             libglapi libglapi-dev \
             libgbm libgbm-dev \
             libgles1-mesa libgles1-mesa-dev \
             libgles2-mesa libgles2-mesa-dev \
             libgles3-mesa libgles3-mesa-dev \
             libwayland-egl libwayland-egl-dev \
             libxatracker libxatracker-dev \
             mesa-megadriver mesa-vulkan-drivers \
            "

# they don't get Debian-renamed (which would remove the -mesa suffix), and
# RPROVIDEs/RCONFLICTs on the generic libgl name.
python __anonymous() {
    pkgconfig = (d.getVar('PACKAGECONFIG') or "").split()
    for p in (("egl", "libegl", "libegl1"),
              ("dri", "libgl", "libgl1"),
              ("gles", "libgles1", "libglesv1-cm1"),
              ("gles", "libgles2", "libglesv2-2"),
              ("gles", "libgles3",)):
        if not p[0] in pkgconfig:
            continue
        fullp = p[1] + "-mesa"
        pkgs = " ".join(p[1:])
        d.setVar("DEBIAN_NOAUTONAME_" + fullp, "1")
        d.appendVar("RREPLACES_" + fullp, pkgs)
        d.appendVar("RPROVIDES_" + fullp, pkgs)
        d.appendVar("RCONFLICTS_" + fullp, pkgs)

        d.appendVar("RRECOMMENDS_" + fullp, " mesa-megadriver")

        # For -dev, the first element is both the Debian and original name
        fullp += "-dev"
        pkgs = p[1] + "-dev"
        d.setVar("DEBIAN_NOAUTONAME_" + fullp, "1")
        d.appendVar("RREPLACES_" + fullp, pkgs)
        d.appendVar("RPROVIDES_" + fullp, pkgs)
        d.appendVar("RCONFLICTS_" + fullp, pkgs)
}

python mesa_populate_packages() {
    pkgs = ['mesa', 'mesa-dev', 'mesa-dbg']
    for pkg in pkgs:
        d.setVar("RPROVIDES_%s" % pkg, pkg.replace("mesa", "mesa-dri", 1))
        d.setVar("RCONFLICTS_%s" % pkg, pkg.replace("mesa", "mesa-dri", 1))
        d.setVar("RREPLACES_%s" % pkg, pkg.replace("mesa", "mesa-dri", 1))

    import re
    dri_drivers_root = oe.path.join(d.getVar('PKGD'), d.getVar('libdir'), "dri")
    if os.path.isdir(dri_drivers_root):
        dri_pkgs = os.listdir(dri_drivers_root)
        lib_name = d.expand("${MLPREFIX}mesa-megadriver")
        for p in dri_pkgs:
            m = re.match('^(.*)_dri\.so$', p)
            if m:
                pkg_name = " ${MLPREFIX}mesa-driver-%s" % legitimize_package_name(m.group(1))
                d.appendVar("RPROVIDES_%s" % lib_name, pkg_name)
                d.appendVar("RCONFLICTS_%s" % lib_name, pkg_name)
                d.appendVar("RREPLACES_%s" % lib_name, pkg_name)

    pipe_drivers_root = os.path.join(d.getVar('libdir'), "gallium-pipe")
    do_split_packages(d, pipe_drivers_root, '^pipe_(.*)\.so$', 'mesa-driver-pipe-%s', 'Mesa %s pipe driver', extra_depends='')
}

PACKAGESPLITFUNCS_prepend = "mesa_populate_packages "

PACKAGES_DYNAMIC += "^mesa-driver-.*"

FILES_${PN} += "${sysconfdir}/drirc"
FILES_mesa-megadriver = "${libdir}/dri/*"
FILES_mesa-vulkan-drivers = "${libdir}/libvulkan_*.so ${datadir}/vulkan"
FILES_libegl-mesa = "${libdir}/libEGL.so.*"
FILES_libgbm = "${libdir}/libgbm.so.*"
FILES_libgles1-mesa = "${libdir}/libGLESv1*.so.*"
FILES_libgles2-mesa = "${libdir}/libGLESv2.so.*"
FILES_libgl-mesa = "${libdir}/libGL.so.*"
FILES_libglapi = "${libdir}/libglapi.so.*"
FILES_libosmesa = "${libdir}/libOSMesa.so.*"
FILES_libwayland-egl = "${libdir}/libwayland-egl.so.*"
FILES_libxatracker = "${libdir}/libxatracker.so.*"

FILES_${PN}-dev = "${libdir}/pkgconfig/dri.pc ${includedir}/vulkan"
FILES_libegl-mesa-dev = "${libdir}/libEGL.* ${includedir}/EGL ${includedir}/KHR ${libdir}/pkgconfig/egl.pc"
FILES_libgbm-dev = "${libdir}/libgbm.* ${libdir}/pkgconfig/gbm.pc ${includedir}/gbm.h"
FILES_libgl-mesa-dev = "${libdir}/libGL.* ${includedir}/GL ${libdir}/pkgconfig/gl.pc"
FILES_libglapi-dev = "${libdir}/libglapi.*"
FILES_libgles1-mesa-dev = "${libdir}/libGLESv1*.* ${includedir}/GLES ${libdir}/pkgconfig/glesv1*.pc"
FILES_libgles2-mesa-dev = "${libdir}/libGLESv2.* ${includedir}/GLES2 ${libdir}/pkgconfig/glesv2.pc"
FILES_libgles3-mesa-dev = "${includedir}/GLES3"
FILES_libosmesa-dev = "${libdir}/libOSMesa.* ${includedir}/GL/osmesa.h ${libdir}/pkgconfig/osmesa.pc"
FILES_libwayland-egl-dev = "${libdir}/pkgconfig/wayland-egl.pc ${libdir}/libwayland-egl.*"
FILES_libxatracker-dev = "${libdir}/libxatracker.so ${libdir}/libxatracker.la \
                          ${includedir}/xa_tracker.h ${includedir}/xa_composite.h ${includedir}/xa_context.h \
                          ${libdir}/pkgconfig/xatracker.pc"

#SOLIBS = ".so.*"
INSANE_SKIP_libegl-mesa += "dev-so ldflags file-rdeps build-deps file-rdeps"
INSANE_SKIP_libgles1-mesa += "dev-so ldflags build-deps file-rdeps"
INSANE_SKIP_libgles2-mesa += "dev-so ldflags build-deps file-rdeps"
INSANE_SKIP_libgles3-mesa += "dev-so ldflags build-deps file-rdeps"
INSANE_SKIP_libvulkan += "dev-so ldflags build-deps file-rdeps"
INSANE_SKIP_libgbm += "dev-so ldflags file-rdeps build-deps file-rdeps"
INSANE_SKIP_libgl-mesa += "dev-so ldflags build-deps file-rdeps"
INSANE_SKIP_libglapi += "dev-so ldflags build-deps file-rdeps"
INSANE_SKIP_libdri += "dev-so ldflags build-deps file-rdeps"
INSANE_SKIP_libwayland-egl += "dev-so ldflags build-deps file-rdeps"
INSANE_SKIP_libgl += "dev-so ldflags build-deps file-rdeps"
INSANE_SKIP_mesa-megadriver += "dev-so ldflags build-deps file-rdeps"

INSANE_SKIP_${PN} += "already-stripped ldflags build-deps file-rdeps"
INSANE_SKIP_${PN}-dev += "already-stripped ldflags build-deps file-rdeps"

