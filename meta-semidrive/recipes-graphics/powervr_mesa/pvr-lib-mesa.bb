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
do_install[nostamp] += "1"

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
    cp -rp ${S}/include/vulkan/* ${D}/${includedir}/vulkan

    install -d ${D}${bindir}
    install -m 755 ${S}/${GPU_HW}/bin/* ${D}${bindir}

    if [ "${MACHINE_GPU_WITHOUT_OPENCL}" = "1" ]; then
       rm -rf ${D}${libdir}/libufwriter.so*
       rm -rf ${D}${libdir}/libVK_IMG.so*
       rm -rf ${D}${libdir}/libOpenCL.so*
       rm -rf ${D}${libdir}/libvulkan.so*
       rm -rf ${D}${libdir}/libPVROCL.so*
    fi

}


# let the build system extends the FILESPATH file search path
FILESEXTRAPATHS_prepend := "${TOPDIR}/../prebuilt/gpu/powervr_mesa:"

PROVIDES = " \
    virtual/pvr \
    "

PACKAGES =+ "\
             libvulkan \
             ${PN}-tests \
            "

FILES_${PN} += "\
                ${nonarch_base_libdir}/firmware/rgx.fw.* \
                ${nonarch_base_libdir}/firmware/rgx.sh.* \
                ${libdir}/libGLESv1_CM_PVR_MESA.so* \
                ${libdir}/libGLESv2_PVR_MESA.so* \
                ${libdir}/libglslcompiler.so* \
                ${libdir}/libpvr_dri_support.so* \
                ${libdir}/libsrv_um.so* \
                ${libdir}/libsutu_display.so* \
                ${libdir}/libusc.so \
                "

FILES_libvulkan = "${libdir}/libvulkan* \
                ${libdir}/libVK_IMG.so* \
                ${libdir}/libufwriter.so* \
                ${libdir}/libOpenCL.so* \
                ${libdir}/libPVROCL.so* \
                "

#SOLIBS = ".so.*"
INSANE_SKIP_libvulkan += "dev-so"

# Include these files in the install on the target
FILES_${PN}-dev += "${includedir}/vulkan \
                   "

FILES_${PN}-tests += "${bindir}/* \
                      /usr/local/share/pvr/shaders/* \
                      "

INSANE_SKIP_libvulkan += "dev-so ldflags file-rdeps"
INSANE_SKIP_${PN}-tests += "ldflags file-rdeps"
# the firmware/rgx.fw.* is in MIPS arch, but current compiler requires aarch64
# to skip such QA Issue: Architecture did not match (MIPS, expected AArch64)
INSANE_SKIP_${PN} += "arch"
# If set to "1", causes the build to not strip binaries in resulting packages
INHIBIT_PACKAGE_STRIP = "1"
# To prevent the build system from splitting out debug information during packaging
INHIBIT_PACKAGE_DEBUG_SPLIT  = "1"

FILES_SOLIBSDEV = ""
INSANE_SKIP_${PN} += "already-stripped ldflags file-rdeps dev-so"
