#
# Recipe for Little kernel

LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

inherit externalsrc

EXTERNALSRC = "${TOPDIR}/../source/atf"
EXTERNALSRC_BUILD = "${EXTERNALSRC}/arm-trusted-firmware"

EXTRA_OEMAKE = ""

CFLAGS=""
LDFLAGS=""

do_configure() {
    echo "skip this step"
}

REPO_SRC_DIR = "${TOPDIR}/../source"

do_compile[nostamp] += "1"
do_deploy[nostamp] += "1"

do_compile() {
    make sml ${MACHINE_ATFARG}
}

inherit deploy
do_deploy() {
    install -m 0644 sml/sml.bin ${DEPLOYDIR}/sml.bin
}

addtask do_deploy after do_compile
do_buildclean() {
    make clean
    rm -rf sml/*
}

##FILES_${PN} = "${bindir}/*"
