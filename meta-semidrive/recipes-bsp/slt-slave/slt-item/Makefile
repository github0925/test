
OBJS = slt_demo.o
LIB = slt_demo.so

all: $(LIB)

$(LIB): $(OBJS)
	$(CXX) $(LDFLAGS) -shared -o $(LIB) $(OBJS)

clean:
	rm -f $(OBJS)

%.o: %.c
	$(CC) -fpic -c $(CFLAGS) -o $@ $<

