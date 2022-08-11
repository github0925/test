
OBJS = slt_emmc.o
LIB = slt_emmc.so

LDLIBS += \
	-lcrypto \

all: $(LIB)

$(LIB): $(OBJS)
	$(CXX) $(LDFLAGS) -shared -o $(LIB) $(OBJS) $(LDLIBS)

clean:
	rm -f $(OBJS)
	rm -f $(LIB)

%.o: %.c
	$(CC) -fpic -c $(CFLAGS) -o $@ $<

