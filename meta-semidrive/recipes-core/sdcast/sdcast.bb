SUMMARY = "SdCast"
DESCRIPTION = "SdCast lib for display sharing"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b30cbe0b980e98bfd9759b1e6ba3d107"

DEPENDS = "libdrm"

SRC_URI = "\
	file://LICENSE \
	file://Makefile \
	file://src/DisplayController.cpp\
	file://src/DrmDisplay.cpp \
	file://src/FrameReceiver.cpp \
	file://src/Socket.cpp \
	file://src/Surface.cpp \
	file://src/utils.cpp \
	file://include/Communication.h \
	file://include/debug.h \
	file://include/DisplayController.h \
	file://include/DrmDisplay.h \
	file://include/FrameReceiver.h \
	file://include/MyThread.h \
	file://include/Socket.h \
	file://include/Surface.h \
	file://include/SurfaceInfo_public.h \
	file://include/utils.h \
	file://test/main.cpp \
	"

S = "${WORKDIR}"

#QA Issue: -dev package contains non-symlink .so
FILES_SOLIBSDEV = ""

FILES_${PN} = "${libdir}/libSdCast.so \
			   ${bindir}/sdcast_test"

#QA Issue: No GNU_HASH in the elf binary
INSANE_SKIP_${PN} = "ldflags"

do_compile() {
    make MACHINE=${MACHINE}
}

do_install () {
	install -d ${D}${libdir}
	install -d ${D}${bindir}
    cp -a --no-preserve=ownership ${S}/libSdCast.so ${D}${libdir}
	install -m 0755 sdcast_test ${D}${bindir}/sdcast_test
}