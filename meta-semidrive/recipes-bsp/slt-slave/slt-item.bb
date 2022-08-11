SUMMARY = "slt item"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b30cbe0b980e98bfd9759b1e6ba3d107"

SRC_URI = "\
	file://LICENSE \
	file://Makefile \
	file://slt_demo.c\
	"

S = "${WORKDIR}"

FILES_${PN} = "\
	/lib/slt/slt_demo.so\
"

do_install () {
	install -d ${D}/lib/slt
	install -m 0755 slt_demo.so ${D}/lib/slt/slt_demo.so
}
