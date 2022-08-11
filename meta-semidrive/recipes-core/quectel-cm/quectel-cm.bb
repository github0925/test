
SUMMARY = "quectel-CM"

LICENSE = "CLOSED"

SRC_URI = " \
	file://Makefile \
	file://GobiNetCM.c \
	file://main.c \
	file://MPQCTL.h \
	file://MPQMI.h \
	file://MPQMUX.c \
	file://MPQMUX.h \
	file://QMIThread.c \
	file://QMIThread.h \
	file://QmiWwanCM.c \
	file://udhcpc.c \
	file://util.c \
	file://util.h \
	file://qmap_bridge_mode.c \
	file://mbim-cm.c \
	file://device.c \
"

S = "${WORKDIR}"

FILES_${PN} = "/usr/bin/quectel-cm"

do_compile() {
	make 
}

do_install () {
	install -d ${D}/usr/bin
	install -m 0755 quectel-cm ${D}/usr/bin/quectel-cm
}
