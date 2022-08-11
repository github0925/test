SUMMARY = "a/b slots update library"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b30cbe0b980e98bfd9759b1e6ba3d107"

SRC_URI = "\
	file://LICENSE \
	file://Makefile \
	file://ab_partition_parser.c \
	file://partition_parser.c \
	file://slots_parse.c \
	file://storage_device.c \
	file://crc32.c \
	file://sample.c \
	file://storage_dev_ospi.c \
	file://include/ab_partition_parser.h \
	file://include/crc32.h \
	file://include/partition_parser.h \
	file://include/slots_parse.h \
	file://include/storage_device.h \
	file://include/storage_dev_ospi.h \
	file://include/system_cfg.h \
	"

S = "${WORKDIR}"

FILES_${PN} = "\
	/usr/lib/libslots_parse.so\
"
FILES_SOLIBSDEV = ""

do_install () {
	install -d ${D}/usr/lib
	install -m 0755 libslots_parse.so ${D}/usr/lib/libslots_parse.so
}
