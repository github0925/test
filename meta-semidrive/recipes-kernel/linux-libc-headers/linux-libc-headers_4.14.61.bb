require recipes-kernel/linux-libc-headers/linux-libc-headers.inc

#do_configure[depends] += "virtual/kernel:do_shared_workdir"

SRC_URI[md5sum] = "418c38237a2ac085171ecdfec133af91"
SRC_URI[sha256sum] = "0d0ec521a771e7d393f25b789a06d9af6e2a5a4837fadc04e7048e03b41c70e8"

LINUX_VERSION ?= "4.14.61"

do_install_armmultilib_prepend() {
    touch ${D}${includedir}/asm/bpf_perf_event.h
}