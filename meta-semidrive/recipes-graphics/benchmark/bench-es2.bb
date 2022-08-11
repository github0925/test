SUMMARY = "GFX ES2 benchmark"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=f7b6e163036f44f4281e15d49d77ecc6"

SRC_URI = "\
	file://LICENSE \
	file://bench_es2.c \
	"

S = "${WORKDIR}"

LDFLAGS_append = " -Wl,--no-as-needed -lEGL -lGLESv2 -fPIC"
DEPENDS = "virtual/libgles2 virtual/egl"
##RDEPENDS_${PN} = "virtual/libgles2 virtual/egl"

do_compile() {
	${CC} ${LDFLAGS} bench_es2.c -o bench_es2
}

FILES_${PN} = "\
	/usr/bin/bench_es2 \
"

do_install () {
	install -d ${D}/usr/bin
	install -m 0755 bench_es2 ${D}/usr/bin
}
