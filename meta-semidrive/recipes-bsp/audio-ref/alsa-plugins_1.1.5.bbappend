FILESEXTRAPATHS_prepend := "${THISDIR}/alsaplugins:"
SRC_URI += "file://0001-sd-remote.patch"
PACKAGECONFIG[arcamav] = "--enable-arcamav"
