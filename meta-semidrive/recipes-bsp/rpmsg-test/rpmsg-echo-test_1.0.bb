SUMMARY = "RPMsg examples: echo test demo"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b30cbe0b980e98bfd9759b1e6ba3d107"

SRC_URI = "\
	file://LICENSE \
	file://Makefile \
	file://src/echo_test.c \
	file://src/cansend.c \
	file://src/candump.c \
	file://inc/can_config.h \
	"

S = "${WORKDIR}"

FILES_${PN} = "\
	/usr/bin/echo_test\
	/usr/bin/property \
	/usr/bin/candump \
	/usr/bin/cansend \
"

do_compile() {
    make MACHINE=${MACHINE} SUPPORT_BOARD_DIAG=${SUPPORT_BOARD_DIAG}
}

do_install () {
	install -d ${D}/usr/bin
	install -m 0755 echo_test ${D}/usr/bin/echo_test
	ln -srf ${D}/usr/bin/echo_test ${D}/usr/bin/property

	install -m 0755 candump ${D}/usr/bin/candump
	install -m 0755 cansend ${D}/usr/bin/cansend
}
