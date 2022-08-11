SUMMARY = "slt slave"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b30cbe0b980e98bfd9759b1e6ba3d107"

SRC_URI = "\
	file://LICENSE \
	file://Makefile \
	file://slt_main.c \
	file://slt_main.h \
	file://slt_message.c \
	file://slt_message.h \
	file://slt_test.c \
	file://slt_test.h \
	file://slt_config.c \
	file://slt_config.h \
	"

S = "${WORKDIR}"

FILES_${PN} = "\
	/usr/bin/slt\
"

do_install () {
	install -d ${D}/usr/bin
	install -m 0755 slt ${D}/usr/bin/slt
}
