APP := sdpe_test_app
SDPE_ETH_OBJ := sdpe_eth.o
CFLAGS += -Wall -Werror

all: $(APP)

$(APP): $(SDPE_ETH_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS) -lpthread

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

clean:
	rm -rf $(APP) *.o
