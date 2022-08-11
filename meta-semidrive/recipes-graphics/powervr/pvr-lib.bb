SUMMARY = "Install prebuilt graphics and OpenGLES library "
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

SRC_URI = "file://include \
           file://gm9446 \
           file://gm9226 \
           "

S = "${WORKDIR}"
DEPENDS = "libdrm"


PV = "1.13"

## prebuilt library don't need following steps
do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_install () {
    FIRMWARE_INSTALL_DIR=${D}${nonarch_base_libdir}/firmware
    SHADERS_INSTALL_DIR=${D}/usr/local/share/pvr/shaders

    # Select gpu type 9226 or 9446 (default)
    if [ "x${MACHINE_GPU_REV}" != "x" ]; then
        GPU_HW=${MACHINE_GPU_REV}
    else
        GPU_HW="gm9446"
    fi

    # Select gpu type 9226 or 9446 (default)
    if [ "x${GPU_VZ_CTRL}" != "x" ]; then
        GPU_VZ=${GPU_VZ_CTRL}
    else
        GPU_VZ="vz"
    fi

    install -d -m 0755 ${FIRMWARE_INSTALL_DIR}
    install -m 644 ${S}/${GPU_HW}/firmware/${GPU_VZ}/rgx.fw.* ${FIRMWARE_INSTALL_DIR}
    install -m 644 ${S}/${GPU_HW}/firmware/${GPU_VZ}/rgx.sh.* ${FIRMWARE_INSTALL_DIR}

    install -d -m 0755 ${SHADERS_INSTALL_DIR}
    install -m 644 ${S}/${GPU_HW}/shaders/* ${SHADERS_INSTALL_DIR}

    install -d ${D}${libdir}
    cp -rd ${S}/${GPU_HW}/lib/*.so* ${D}${libdir}

    # Install headers
    install -d ${D}/${includedir}
    install -d ${D}/${includedir}/vulkan
    cp -rp ${S}/include/* ${D}/${includedir}
    cp -rp ${S}/include/vulkan/* ${D}/${includedir}/vulkan

    install -d ${D}${bindir}
    install -m 755 ${S}/${GPU_HW}/bin/* ${D}${bindir}

    if [ "${MACHINE_GPU_WITHOUT_OPENCL}" = "1" ]; then
       rm -rf ${D}${libdir}/libufwriter.so*
       rm -rf ${D}${libdir}/libVK_IMG.so*
       rm -rf ${D}${libdir}/libOpenCL.so*
       rm -rf ${D}${libdir}/libvulkan.so*
       rm -rf ${D}${libdir}/libPVROCL.so*
       rm -rf ${D}${bindir}/vkbonjour
       rm -rf ${D}${bindir}/ocl_unit_test
    fi

}


# let the build system extends the FILESPATH file search path
FILESEXTRAPATHS_prepend := "${TOPDIR}/../prebuilt/gpu/powervr:"

PACKAGECONFIG = "${@bb.utils.contains('DISTRO_FEATURES', 'opengl', 'egl gles', '', d)} \
		   "

PROVIDES = " \
    ${@bb.utils.contains('PACKAGECONFIG', 'gles', 'virtual/libgles1 virtual/libgles2', '', d)} \
    ${@bb.utils.contains('PACKAGECONFIG', 'egl', 'virtual/egl', '', d)} \
    virtual/pvr \
    "

PACKAGES =+ "libegl-pvr libegl-pvr-dev \
             libgles1-pvr libgles1-pvr-dev \
             libgles2-pvr libgles2-pvr-dev \
             libgles3-pvr libgles3-pvr-dev \
             libvulkan \
             ${PN}-tests \
            "

# Add dependency so that GLES3 header don't need to be added manually
RDEPENDS_libgles2-pvr-dev += "libgles3-pvr-dev"

#RPROVIDES_${PN} = "opengles"

FILES_${PN} += "\
                ${nonarch_base_libdir}/firmware/rgx.fw.* \
                ${nonarch_base_libdir}/firmware/rgx.sh.* \
                ${libdir}/libglslcompiler.so \
                ${libdir}/libIMGegl.so \
                ${libdir}/libpvrNULLDRM_WSEGL.so \
                ${libdir}/libsrv_um.so \
                ${libdir}/libsutu_display.so \
                ${libdir}/libgbm.so \
                ${libdir}/libusc.so \
                ${libdir}/libPVROCL.so \
                ${libdir}/libsutu_display.so \
                ${libdir}/libufwriter.so \
                ${libdir}/libOpenCL.so \
                "

FILES_libegl-pvr = "${libdir}/libEGL.so* \
                /usr/local/share/pvr/shaders/*"

FILES_libvulkan = "${libdir}/libvulkan.so* \
                ${libdir}/libvulkan-1.so \
                ${libdir}/libVK_IMG.so* \
                ${libdir}/libufwriter.so* \
                ${libdir}/libPVROCL.so* \
                "
FILES_libgles1-pvr = "${libdir}/libGLES_CM.so*"
FILES_libgles2-pvr = "${libdir}/libGLESv2.so*"
FILES_libgles3-pvr = "${libdir}/libGLESv2.so*"

#SOLIBS = ".so.*"
INSANE_SKIP_libegl-pvr += "dev-so"
INSANE_SKIP_libgles1-pvr += "dev-so"
INSANE_SKIP_libgles2-pvr += "dev-so"
INSANE_SKIP_libgles3-pvr += "dev-so"
INSANE_SKIP_libvulkan += "dev-so"

# Include these files in the install on the target
FILES_${PN}-dev += "${includedir}/vulkan"
FILES_libegl-pvr-dev = "${libdir}/libEGL.so* ${includedir}/EGL ${includedir}/KHR"
FILES_libgles1-pvr-dev = "${libdir}/libGLES_CM.so* ${includedir}/GLES"
FILES_libgles2-pvr-dev = "${libdir}/libGLESv2.so* ${includedir}/GLES2"
FILES_libgles3-pvr-dev = "${includedir}/GLES3"

FILES_${PN}-tests += "${bindir}/*"
RDEPENDS_${PN}-tests += " libgles1-pvr libegl-pvr libgles2-pvr "

# the firmware/rgx.fw.* is in MIPS arch, but current compiler requires aarch64
# to skip such QA Issue: Architecture did not match (MIPS, expected AArch64)
INSANE_SKIP_${PN} += "arch"
# If set to "1", causes the build to not strip binaries in resulting packages
INHIBIT_PACKAGE_STRIP = "1"
# To prevent the build system from splitting out debug information during packaging
INHIBIT_PACKAGE_DEBUG_SPLIT  = "1"

FILES_SOLIBSDEV = ""
INSANE_SKIP_${PN} += "already-stripped ldflags file-rdeps dev-so"
