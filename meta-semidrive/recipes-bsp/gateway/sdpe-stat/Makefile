APP := sdpe_stat
SDPE_STAT_OBJ := sdpe_stat.o
CFLAGS += -Wall -Werror

all: $(APP)

$(APP): $(SDPE_STAT_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS) -lpthread

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

clean:
	rm -rf $(APP) *.o
