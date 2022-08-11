APP := quectel-cm

APP_OBJS := QmiWwanCM.o \
            GobiNetCM.o \
			main.o \
			MPQMUX.o \
			QMIThread.o \
			util.o \
			udhcpc.o \
			qmap_bridge_mode.o mbim-cm.o device.o 
# Add any other object files to this list below

all: $(APP)

$(APP): $(APP_OBJS)
	$(CXX) $(LDFLAGS) -o $(APP) $(APP_OBJS) $(LDLIBS) -lrt -ldl -lpthread

clean:
	rm -rf $(APP) *.o

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
