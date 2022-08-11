
APP = ce-test
APP_OBJS = ce_test.o rsa_data.o

# Add any other object files to this list below

all: $(APP)

$(APP): $(APP_OBJS)
	$(CXX) $(LDFLAGS) -o $(APP) $(APP_OBJS) $(LDLIBS) -lpthread

clean:
	rm -rf $(APP) *.o

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

