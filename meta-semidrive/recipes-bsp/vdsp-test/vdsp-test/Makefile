APP = vdsp-test
APP_OBJS = vdsptest.o \
			g2d.o \
			xrp_linux_native.o

XRP_INCS = -I xrp-linux-native/include/ \
			-I g2d/include/

# Add any other object files to this list below

all: $(APP)

$(APP): $(APP_OBJS)
	$(CXX) $(LDFLAGS) -o $(APP) $(APP_OBJS) $(LDLIBS) -L. -ldrm -lpthread

vdsptest.o: vdsptest.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $< $(XRP_INCS)

xrp_linux_native.o: xrp-linux-native/xrp_linux_native.c
	$(CC) -c $(CFLAGS) -o $@ $< $(XRP_INCS)

g2d.o: g2d/g2dapi.c
	$(CC) -c $(CFLAGS) -o $@ $< $(XRP_INCS)


clean:
	rm -rf $(APP) *.o

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $< -Iinclude

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $< -Iinclude
