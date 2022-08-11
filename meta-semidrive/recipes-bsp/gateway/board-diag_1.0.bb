SUMMARY = "ii4 board diagnostic service."

LICENSE = "CLOSED"

DEPENDS += " \
	libgpiod \
	openssl \
"

SRC_URI = " \
	file://Makefile \
	file://src/board_diag.c \
	file://src/cfg_g9x_ii4.c \
	file://src/cfg_g9x_ref.c \
	file://src/debug.c \
	file://src/func_emmc.c \
	file://src/func_eth.c \
	file://src/func_gpio.c \
	file://src/gpio-ops.c \
	file://src/rpmsg_ops.c \
	file://src/sw_timer.c \
	file://src/func_usb.c \
	file://src/func_capt.c \
	file://src/socket_udp_server.c \
	file://src/socket_udp_client.c \
	file://src/socket_tcp_server_epoll.c \
	file://src/socket_tcp_client.c \
	file://src/eth_port_socket.c \
	file://src/eth_port_vlan.c \
	file://src/filter_lib.c \
	file://inc/board_diag.h \
	file://inc/cfg.h \
	file://inc/debug.h \
	file://inc/func_capt.h \
	file://inc/func_emmc.h \
	file://inc/func_eth.h \
	file://inc/func_gpio.h \
	file://inc/func_usb.h \
	file://inc/gpio-ops.h \
	file://inc/rpmsg_ops.h \
	file://inc/sdrv_types.h \
	file://inc/socket.h \
	file://inc/sw_timer.h \
"

S = "${WORKDIR}"

FILES_${PN} = "/usr/bin/board_diag"

do_compile() {
	make MACHINE=${MACHINE} SUPPORT_BOARD_DIAG=${SUPPORT_BOARD_DIAG}
}

do_install () {
	install -d ${D}/usr/bin
	install -m 0755 board_diag ${D}/usr/bin/board_diag
}
