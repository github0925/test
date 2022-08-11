APP := echo_test cansend candump

ECHO_TEST_OBJ  := src/echo_test.o
CAN_DUMP_OBJ   := src/candump.o
CAN_SEND_OBJ   := src/cansend.o

CFLAGS += -Wall -Werror

all: $(APP)

echo_test: $(ECHO_TEST_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS) -lpthread

candump: $(CAN_DUMP_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

cansend: $(CAN_SEND_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^ -I inc/

clean:
	rm -rf $(APP) src/*.o
