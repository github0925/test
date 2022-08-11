SUMMARY = "Gateway sample applications."

LICENSE = "CLOSED"

SRC_URI = " \
	file://Makefile \
	file://doip_test.c \
"

S = "${WORKDIR}"

FILES_${PN} = "/usr/bin/doip_test"

do_compile() {
	make MACHINE=${MACHINE}
}

do_install () {
	install -d ${D}/usr/bin
	install -m 0755 doip_test ${D}/usr/bin/doip_test
}
