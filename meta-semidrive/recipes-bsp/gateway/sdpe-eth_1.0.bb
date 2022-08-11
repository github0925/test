SUMMARY = "SDPE CAN/LIN to Ethernet routing sample."

LICENSE = "CLOSED"

SRC_URI = " \
	file://Makefile \
	file://sdpe_eth.c \
	file://../common/ring_buffer.h \
	file://sdpe_test.sh \
"

S = "${WORKDIR}"

FILES_${PN} = " \
	/usr/bin/sdpe_test_app \
	/usr/bin/sdpe_test \
"

do_compile() {
	make MACHINE=${MACHINE}
}

do_install () {
	install -d ${D}/usr/bin
	install -m 0755 sdpe_test_app ${D}/usr/bin/sdpe_test_app
	install -m 0755 sdpe_test.sh ${D}/usr/bin/sdpe_test
}
