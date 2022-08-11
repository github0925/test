SUMMARY = "GNSS TEST"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=f7b6e163036f44f4281e15d49d77ecc6"

SRC_URI = "\
	file://LICENSE \
	file://Makefile \
	file://gnss-test.cpp \
	"

S = "${WORKDIR}"


LDFLAGS_append = " -Wl,--no-as-needed -lpthread -fPIC"

FILES_${PN} = "\
	/usr/bin/gnss-test \
"

do_install () {
	install -d ${D}/usr/bin
	install -m 0755 gnss-test ${D}/usr/bin
}
