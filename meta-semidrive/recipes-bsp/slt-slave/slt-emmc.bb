SUMMARY = "slt emmc"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b30cbe0b980e98bfd9759b1e6ba3d107"

DEPENDS += " openssl \
	"

SRC_URI = "\
	file://LICENSE \
	file://Makefile \
	file://slt_emmc.c\
	"

S = "${WORKDIR}"

FILES_${PN} = "\
	/lib/slt/slt_emmc.so\
	"

do_compile() {

    make MACHINE=${MACHINE}
}

do_install () {
	install -d ${D}/lib/slt
	install -m 0755 slt_emmc.so ${D}/lib/slt/slt_emmc.so
}
