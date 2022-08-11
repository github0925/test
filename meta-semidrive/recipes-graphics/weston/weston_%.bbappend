FILESEXTRAPATHS_prepend := "${THISDIR}/patch:"

SRC_URI_append := "file://fix-multi-touchscreen-issue.patch"
