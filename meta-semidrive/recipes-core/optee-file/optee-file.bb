SUMMARY = "optee file"
DESCRIPTION = "optee client lib, supplicant ca ta img file"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI = "file://tee-supplicant \
           file://optee_example_hello_world \
           file://libteec.so.1 \
           file://8aaaf200-2450-11e4-abe2-0002a5d5c51b.ta \
           file://COPYING \
"

S = "${WORKDIR}"
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
INSANE_SKIP_${PN} += "installed-vs-shippedi ldflags"

do_install () {

    install -d ${D}${bindir}
    install -m 0755 tee-supplicant ${D}${bindir}/
    install -d ${D}${bindir}
    install -m 0755 optee_example_hello_world ${D}${bindir}/
    install -d ${D}${libdir}/
    install -m 0755 libteec.so.1 ${D}${libdir}/
    MODULE_DIR=${D}${nonarch_base_libdir}/optee_armtz/
    install -d ${MODULE_DIR}
    install -m 755 8aaaf200-2450-11e4-abe2-0002a5d5c51b.ta ${MODULE_DIR}
}

FILES_${PN} += "/lib \
                /lib/optee_armtz/ \
                /lib/optee_armtz/8aaaf200-2450-11e4-abe2-0002a5d5c51b.ta \
"

