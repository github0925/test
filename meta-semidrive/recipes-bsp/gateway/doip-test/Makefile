APP := doip_test
DOIP_SAMPLE_OBJ  := doip_test.o
CFLAGS += -Wall -Werror

all: $(APP)

$(APP): $(DOIP_SAMPLE_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS) -lpthread

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

clean:
	rm -rf $(APP) *.o
