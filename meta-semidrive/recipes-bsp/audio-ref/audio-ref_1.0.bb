SUMMARY = "Alsa init configuration"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=5b1711053a460f05f5fa68fe017c89b0"

SRC_URI = "\
	file://LICENSE \
	file://fw/ak7738_cram_dsp1_data1.bin \
	file://fw/ak7738_cram_dsp1_data2.bin \
	file://fw/ak7738_cram_dsp1_data3.bin \
	file://fw/ak7738_cram_dsp1_data5.bin \
	file://fw/ak7738_cram_dsp2_data1.bin \
	file://fw/ak7738_cram_dsp2_data2.bin \
	file://fw/ak7738_cram_dsp2_data3.bin \
	file://fw/ak7738_cram_dsp2_data5.bin \
	file://fw/ak7738_cram_dsp3_data5.bin \
	file://fw/ak7738_ofreg_dsp1_data1.bin \
	file://fw/ak7738_ofreg_dsp1_data2.bin \
	file://fw/ak7738_ofreg_dsp1_data3.bin \
	file://fw/ak7738_ofreg_dsp2_data1.bin \
	file://fw/ak7738_ofreg_dsp2_data2.bin \
	file://fw/ak7738_ofreg_dsp2_data3.bin \
	file://fw/ak7738_ofreg_dsp2_data5.bin \
	file://fw/ak7738_pram_dsp1_data1.bin \
	file://fw/ak7738_pram_dsp1_data2.bin \
	file://fw/ak7738_pram_dsp1_data3.bin \
	file://fw/ak7738_pram_dsp1_data5.bin \
	file://fw/ak7738_pram_dsp2_data1.bin \
	file://fw/ak7738_pram_dsp2_data2.bin \
	file://fw/ak7738_pram_dsp2_data3.bin \
	file://fw/ak7738_pram_dsp2_data5.bin \
	file://fw/ak7738_pram_dsp3_data5.bin \
	file://asound.state \
	file://asound.conf \
	file://alsa.sh \
	"
inherit update-rc.d
S = "${WORKDIR}"

INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "alsa.sh"


FILES_${PN} = "\
	/lib/firmware/ak7738_cram_dsp1_data1.bin\
	/lib/firmware/ak7738_cram_dsp1_data2.bin\
	/lib/firmware/ak7738_cram_dsp1_data3.bin\
	/lib/firmware/ak7738_cram_dsp1_data5.bin\
	/lib/firmware/ak7738_cram_dsp2_data1.bin\
	/lib/firmware/ak7738_cram_dsp2_data2.bin\
	/lib/firmware/ak7738_cram_dsp2_data3.bin\
	/lib/firmware/ak7738_cram_dsp2_data5.bin\
	/lib/firmware/ak7738_cram_dsp3_data5.bin\
	/lib/firmware/ak7738_ofreg_dsp1_data1.bin\
	/lib/firmware/ak7738_ofreg_dsp1_data2.bin\
	/lib/firmware/ak7738_ofreg_dsp1_data3.bin\
	/lib/firmware/ak7738_ofreg_dsp2_data1.bin\
	/lib/firmware/ak7738_ofreg_dsp2_data2.bin\
	/lib/firmware/ak7738_ofreg_dsp2_data3.bin\
	/lib/firmware/ak7738_ofreg_dsp2_data5.bin\
	/lib/firmware/ak7738_pram_dsp1_data1.bin\
	/lib/firmware/ak7738_pram_dsp1_data2.bin\
	/lib/firmware/ak7738_pram_dsp1_data3.bin\
	/lib/firmware/ak7738_pram_dsp1_data5.bin\
	/lib/firmware/ak7738_pram_dsp2_data1.bin\
	/lib/firmware/ak7738_pram_dsp2_data2.bin\
	/lib/firmware/ak7738_pram_dsp2_data3.bin\
	/lib/firmware/ak7738_pram_dsp2_data5.bin\
	/lib/firmware/ak7738_pram_dsp3_data5.bin\
	/var/lib/alsa/asound.state_org\
	/etc/asound.conf_org\
	/etc/init.d/alsa.sh\
"

do_install () {
	install -d ${D}/lib/firmware
	install -m 0644 fw/ak7738_cram_dsp1_data1.bin ${D}/lib/firmware/ak7738_cram_dsp1_data1.bin
	install -m 0644 fw/ak7738_cram_dsp1_data2.bin ${D}/lib/firmware/ak7738_cram_dsp1_data2.bin
	install -m 0644 fw/ak7738_cram_dsp1_data3.bin ${D}/lib/firmware/ak7738_cram_dsp1_data3.bin
	install -m 0644 fw/ak7738_cram_dsp1_data5.bin ${D}/lib/firmware/ak7738_cram_dsp1_data5.bin
	install -m 0644 fw/ak7738_cram_dsp2_data1.bin ${D}/lib/firmware/ak7738_cram_dsp2_data1.bin
	install -m 0644 fw/ak7738_cram_dsp2_data2.bin ${D}/lib/firmware/ak7738_cram_dsp2_data2.bin
	install -m 0644 fw/ak7738_cram_dsp2_data3.bin ${D}/lib/firmware/ak7738_cram_dsp2_data3.bin
	install -m 0644 fw/ak7738_cram_dsp2_data5.bin ${D}/lib/firmware/ak7738_cram_dsp2_data5.bin
	install -m 0644 fw/ak7738_cram_dsp3_data5.bin ${D}/lib/firmware/ak7738_cram_dsp3_data5.bin
	install -m 0644 fw/ak7738_ofreg_dsp1_data1.bin ${D}/lib/firmware/ak7738_ofreg_dsp1_data1.bin
	install -m 0644 fw/ak7738_ofreg_dsp1_data2.bin ${D}/lib/firmware/ak7738_ofreg_dsp1_data2.bin
	install -m 0644 fw/ak7738_ofreg_dsp1_data3.bin ${D}/lib/firmware/ak7738_ofreg_dsp1_data3.bin
	install -m 0644 fw/ak7738_ofreg_dsp2_data1.bin ${D}/lib/firmware/ak7738_ofreg_dsp2_data1.bin
	install -m 0644 fw/ak7738_ofreg_dsp2_data2.bin ${D}/lib/firmware/ak7738_ofreg_dsp2_data2.bin
	install -m 0644 fw/ak7738_ofreg_dsp2_data3.bin ${D}/lib/firmware/ak7738_ofreg_dsp2_data3.bin
	install -m 0644 fw/ak7738_ofreg_dsp2_data5.bin ${D}/lib/firmware/ak7738_ofreg_dsp2_data5.bin
	install -m 0644 fw/ak7738_pram_dsp1_data1.bin ${D}/lib/firmware/ak7738_pram_dsp1_data1.bin
	install -m 0644 fw/ak7738_pram_dsp1_data2.bin ${D}/lib/firmware/ak7738_pram_dsp1_data2.bin
	install -m 0644 fw/ak7738_pram_dsp1_data3.bin ${D}/lib/firmware/ak7738_pram_dsp1_data3.bin
	install -m 0644 fw/ak7738_pram_dsp1_data5.bin ${D}/lib/firmware/ak7738_pram_dsp1_data5.bin
	install -m 0644 fw/ak7738_pram_dsp2_data1.bin ${D}/lib/firmware/ak7738_pram_dsp2_data1.bin
	install -m 0644 fw/ak7738_pram_dsp2_data2.bin ${D}/lib/firmware/ak7738_pram_dsp2_data2.bin
	install -m 0644 fw/ak7738_pram_dsp2_data3.bin ${D}/lib/firmware/ak7738_pram_dsp2_data3.bin
	install -m 0644 fw/ak7738_pram_dsp2_data5.bin ${D}/lib/firmware/ak7738_pram_dsp2_data5.bin
	install -m 0644 fw/ak7738_pram_dsp3_data5.bin ${D}/lib/firmware/ak7738_pram_dsp3_data5.bin
	install -d ${D}/var/lib/alsa
	install -d ${D}/etc
	install -m 0644 asound.state ${D}/var/lib/alsa/asound.state_org
	install -m 0644 asound.conf ${D}/etc/asound.conf_org
	install -d ${D}${sysconfdir}/init.d
	install -m 0755 alsa.sh ${D}${sysconfdir}/init.d
}