SUMMARY = "Hardware accelerated JPEG compression/decompression library"
DESCRIPTION = "libjpeg-turbo is a derivative of libjpeg that uses SIMD instructions (MMX, SSE2, NEON) to accelerate baseline JPEG compression and decompression"
HOMEPAGE = "http://libjpeg-turbo.org/"

LICENSE = "BSD-3-Clause"
DEPENDS_append_x86-64_class-target = " nasm-native"
DEPENDS_append_x86_class-target    = " nasm-native"

RDEPENDS_${PN}_aarch64_class-target = " libjpu "
DEPENDS_append_aarch64_class-target = " libjpu "

inherit autotools pkgconfig externalsrc

EXTERNALSRC = "${TOPDIR}/../source/vendor/libjpeg-turbo"
EXTERNALSRC_BUILD = "${WORKDIR}/libjpeg-turbo"


UPSTREAM_CHECK_URI = "http://sourceforge.net/projects/libjpeg-turbo/files/"
UPSTREAM_CHECK_REGEX = "/libjpeg-turbo/files/(?P<pver>(\d+[\.\-_]*)+)/"


PE= "1"

# Drop-in replacement for jpeg
PROVIDES = "jpeg"
RPROVIDES_${PN} += "jpeg"
RREPLACES_${PN} += "jpeg"
RCONFLICTS_${PN} += "jpeg"



# Add nasm-native dependency consistently for all build arches is hard
EXTRA_OECONF_append_class-native = " --without-simd --without-hw"

# Work around missing x32 ABI support
EXTRA_OECONF_append_class-target = " ${@bb.utils.contains("TUNE_FEATURES", "mx32", "--without-simd", "", d)}"

# Work around missing non-floating point ABI support in MIPS
EXTRA_OECONF_append_class-target = " ${@bb.utils.contains("MIPSPKGSFX_FPU", "-nf", "--without-simd", "", d)}"

# Provide a workaround if Altivec unit is not present in PPC
EXTRA_OECONF_append_class-target_powerpc = " ${@bb.utils.contains("TUNE_FEATURES", "altivec", "", "--without-simd", d)}"
EXTRA_OECONF_append_class-target_powerpc64 = " ${@bb.utils.contains("TUNE_FEATURES", "altivec", "", "--without-simd", d)}"

def get_build_time(d):
    if d.getVar('SOURCE_DATE_EPOCH') != None:
        import datetime
        return " --with-build-date="+ datetime.datetime.fromtimestamp(float(d.getVar('SOURCE_DATE_EPOCH'))).strftime("%Y%m%d")
    return ""

EXTRA_OECONF_append_class-target = "${@get_build_time(d)}"

DESCRIPTION_jpeg-tools = "The jpeg-tools package includes client programs to access libjpeg functionality.  These tools allow for the compression, decompression, transformation and display of JPEG files and benchmarking of the libjpeg library."
FILES_jpeg-tools = "${bindir}/*"

DESCRIPTION_libturbojpeg = "A SIMD-accelerated JPEG codec which provides only TurboJPEG APIs"
FILES_libturbojpeg = "${libdir}/libturbojpeg.so.*"

BBCLASSEXTEND = "native"

do_install_append() {
	install -m 0755 -D ${WORKDIR}/libjpeg-turbo/.libs/testlibjpegyuv420 ${D}${bindir}/testlibjpegyuv420
	install -m 0755 -D ${WORKDIR}/libjpeg-turbo/.libs/testlibjpeg ${D}${bindir}/testlibjpeg
}

