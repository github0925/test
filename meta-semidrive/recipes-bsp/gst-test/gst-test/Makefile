
APP = gst_test
APP_OBJS = gst_test.o

# Add any other object files to this list below

all: $(APP)

$(APP): $(APP_OBJS)
	$(CXX) $(LDFLAGS) -o $(APP) -g $(APP_OBJS) $(LDLIBS) -lpthread -lgstreamer-1.0 -lgstbase-1.0 -lgobject-2.0 -lglib-2.0

clean:
	rm -rf $(APP) *.o

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

