SRC_URI_append = "file://fragment.cfg"
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

do_configure_append() {
    if [ "x${GPU_VZ_CTRL}" = "xnonvz" ]; then
        ${S}/scripts/kconfig/merge_config.sh -m -O ${B} ${B}/.config ${WORKDIR}/*.cfg
    fi
}

