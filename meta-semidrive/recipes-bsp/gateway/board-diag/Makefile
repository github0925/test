APP := board_diag

BOARD_DIAG_OBJ := \
	src/board_diag.o \
	src/debug.o \
	src/rpmsg_ops.o \
	src/func_gpio.o \
	src/func_emmc.o \
	src/func_eth.o \
	src/gpio-ops.o \
	src/func_usb.o \
	src/sw_timer.o \
	src/func_capt.o \
	src/socket_udp_server.o \
	src/socket_udp_client.o \
	src/socket_tcp_server_epoll.o \
	src/socket_tcp_client.o \
	src/eth_port_socket.o \
	src/eth_port_vlan.o \
	src/filter_lib.o

ifeq ($(MACHINE), g9x_ii4)
BOARD_DIAG_OBJ +=  src/cfg_g9x_ii4.o
else ifeq ($(MACHINE), g9x_ref)
BOARD_DIAG_OBJ +=  src/cfg_g9x_ref.o
endif

BOARD_DIAG_LIBS := \
	-lgpiod \
	-lcrypto \
	-lpthread

CFLAGS += -Wall -Werror

all: $(APP)

board_diag: $(BOARD_DIAG_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)  $(BOARD_DIAG_LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^ -I inc/

clean:
	rm -rf $(APP) src/*.o
