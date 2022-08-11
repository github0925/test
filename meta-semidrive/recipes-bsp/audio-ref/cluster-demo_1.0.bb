SUMMARY = "Cluster audio demo."

LICENSE = "CLOSED"

SRC_URI = " \
	file://Makefile \
	file://src/cluster_audio.c \
"

S = "${WORKDIR}"

FILES_${PN} = "/usr/bin/cluster_audio"
DEPENDS = "alsa-lib"
CFLAGS_prepend = "-I${STAGING_INCDIR}/alsa-lib/"
do_compile() {
	make cluster_audio
}

do_install () {
	install -d ${D}/usr/bin
	install -m 0755 cluster_audio ${D}/usr/bin/cluster_audio
}
