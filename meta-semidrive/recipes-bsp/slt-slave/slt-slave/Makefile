
APP := slt
APP_OBJS := slt_main.o \
            slt_test.o \
			slt_message.o \
			slt_config.o \

# Add any other object files to this list below

all: $(APP)

$(APP): $(APP_OBJS)
	$(CXX) $(LDFLAGS) -o $(APP) $(APP_OBJS) $(LDLIBS) -lrt -ldl -lpthread

clean:
	rm -rf $(APP) *.o

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

