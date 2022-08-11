.PHONY: clean

#LDFLAGS += -lpthread
CFLAGS += -Wall -lpthread -I./linenoise

SDLOG_TARGET := sdshell
SDLOG_OBJS = sdshell.o linenoise/linenoise.o

$(SDLOG_TARGET): $(SDLOG_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	rm -rf $(SDLOG_OBJS) $(SDLOG_TARGET)
