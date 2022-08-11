SUMMARY = "Audio Test: audio test demo"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=5b1711053a460f05f5fa68fe017c89b0"

SRC_URI = "\
	file://LICENSE \
	file://cov_stereo_8k_16bits.wav \
	file://cov_stereo_16k_16bits.wav \
	file://cov_stereo_32k_16bits.wav \
	file://cov_stereo_44.1k_16bits.wav \
	file://cov_stereo_48k_16bits.wav \
	file://g9_tst.sh \
	"

S = "${WORKDIR}"

FILES_${PN} = "\
	/usr/bin/cov_stereo_8k_16bits.wav\
	/usr/bin/cov_stereo_16k_16bits.wav\
	/usr/bin/cov_stereo_32k_16bits.wav\
	/usr/bin/cov_stereo_44.1k_16bits.wav\
	/usr/bin/cov_stereo_48k_16bits.wav\
	/usr/bin/g9_tst.sh\
"

do_install () {
	install -d ${D}/usr/bin
	install -m 0644 cov_stereo_8k_16bits.wav ${D}/usr/bin/cov_stereo_8k_16bits.wav
	install -m 0644 cov_stereo_16k_16bits.wav ${D}/usr/bin/cov_stereo_16k_16bits.wav
	install -m 0644 cov_stereo_32k_16bits.wav ${D}/usr/bin/cov_stereo_32k_16bits.wav
	install -m 0644 cov_stereo_44.1k_16bits.wav ${D}/usr/bin/cov_stereo_44.1k_16bits.wav
	install -m 0644 cov_stereo_48k_16bits.wav ${D}/usr/bin/cov_stereo_48k_16bits.wav
	install -m 0755 g9_tst.sh ${D}/usr/bin/g9_tst.sh
}
