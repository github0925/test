SUMMARY = "SDPE statistics sample."

LICENSE = "CLOSED"

SRC_URI = " \
	file://Makefile \
	file://sdpe_stat.h \
	file://sdpe_stat.c \
"

S = "${WORKDIR}"

FILES_${PN} = "/usr/bin/sdpe_stat"

do_compile() {
	make MACHINE=${MACHINE}
}

do_install () {
	install -d ${D}/usr/bin
	install -m 0755 sdpe_stat ${D}/usr/bin/sdpe_stat
}
