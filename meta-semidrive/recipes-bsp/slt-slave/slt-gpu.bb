SUMMARY = "slt item"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b30cbe0b980e98bfd9759b1e6ba3d107"

SRC_URI = "\
	file://LICENSE \
	file://Makefile \
	file://slt_gpu.c \
	file://gles3test1/gles3test1.c \
	file://gles3test1/maths.c \
	file://gles3test1/eglutils.c \
	file://ocl_unit_test/ocltest.c \
	file://ocl_unit_test/ocltestmetrics.c \
	file://slt_gpu.h \
	file://gles3test1/maths.h \
	file://gles3test1/eglutils.h \
	file://ocl_unit_test/ocltest.h \
	file://ocl_unit_test/ocltestmetrics.h \
	file://ocl_unit_test/tests/* \
	file://gles3test1_dump_*.dat \
	"

S = "${WORKDIR}"
SHADER_DIR = "/usr/local/data/"


CFLAGS_prepend = "-Wall \
                -Wno-unused-parameter \
                -DLINUX \
                -DSHADER_DIR='"/usr/local/data/"' \
                -I${STAGING_INCDIR} \
                -I${S} \
                "

LDFLAGS_append = " -Wl,--no-as-needed -lEGL -fPIC"
DEPENDS = "virtual/libgles2 virtual/egl"

FILES_${PN} = "\
    /lib/slt/slt_gpu.so \
    /usr/local/data \
    /usr/local/data/gles3test1_dump_*.dat \
    /usr/local/data/*.txt \
"

#FILES_${PN}-dev = "\
#    /usr/local/data/gles3test1_dump_*.dat \
#    /usr/local/data/*.txt \
#"

do_install () {
    DATA_INSTALL_DIR=${D}/usr/local/data

    install -d ${D}/lib/slt
    install -m 0755 slt_gpu.so ${D}/lib/slt/slt_gpu.so

    install -d ${DATA_INSTALL_DIR}
    install -m 644 ${S}/data/* ${DATA_INSTALL_DIR}
}
