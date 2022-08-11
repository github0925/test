SUMMARY = "sdshell: semidrive shell"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b30cbe0b980e98bfd9759b1e6ba3d107"

SRC_URI = "\
	file://LICENSE \
	file://Makefile \
	file://sdshell.c \
	file://sdshell.h \
	file://linenoise/linenoise.c \
	file://linenoise/linenoise.h \
	file://rpmsg_socket.h \
	"

S = "${WORKDIR}"

FILES_${PN} = "\
	/usr/bin/sdshell \
"

LDLIBS_append = "-lpthread"

do_install () {
	install -d ${D}/usr/bin
	install -m 0755 sdshell ${D}/usr/bin/sdshell
}
