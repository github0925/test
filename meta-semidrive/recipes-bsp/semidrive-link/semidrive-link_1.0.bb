SUMMARY = "Semidrive Link examples"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b30cbe0b980e98bfd9759b1e6ba3d107"

SRC_URI = "\
	file://LICENSE \
	file://Makefile \
	file://src/main.cpp \
	file://src/SemidriveLink.cpp \
	file://src/SemidriveLinkThread.cpp \
	file://src/Message512.cpp \
	file://src/Message513.cpp \
	file://src/Message514.cpp \
	file://src/Message521.cpp \
	file://inc/CanData.hpp \
	file://inc/Log.hpp \
	file://inc/Message.hpp \
	file://inc/Message512.hpp \
	file://inc/Message513.hpp \
	file://inc/Message514.hpp \
	file://inc/Message521.hpp \
	file://inc/SemidriveLink.hpp \
	file://inc/SemidriveLinkThread.hpp \
	"

S = "${WORKDIR}"

FILES_${PN} = "\
	/usr/bin/semilink-test \
	"

do_install () {
	install -d ${D}/usr/bin
	install -m 0755 semilink-test ${D}/usr/bin/semilink-test
}
