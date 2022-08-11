SUMMARY = "ce test demo"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b30cbe0b980e98bfd9759b1e6ba3d107"

SRC_URI = "\
	file://LICENSE \
	file://Makefile \
	file://mdiotool.c \
	"

S = "${WORKDIR}"

FILES_${PN} = "\
	/usr/bin/mdiotool\
"

do_install () {
	install -d ${D}/usr/bin
	install -m 0755 mdiotool ${D}/usr/bin/mdiotool
}
