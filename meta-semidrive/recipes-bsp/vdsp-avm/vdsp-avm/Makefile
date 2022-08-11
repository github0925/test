
APP = vdsp-avm
APP_OBJS = vdspavm.o

XRP_INCS = xrp-linux-native/include

# Add any other object files to this list below

all: $(APP)

$(APP): $(APP_OBJS) libxrp.a
	$(CXX) $(LDFLAGS) -o $(APP) $(APP_OBJS) $(LDLIBS) -L. -lxrp -ldrm -lpthread

vdspavm.o: vdspavm.cpp $(XRP_INCS)/xrp_xnnc_ns.h $(XRP_INCS)/xrp_api.h csitest.h
	$(CXX) -c $(CXXFLAGS) -o $@ $< -I$(XRP_INCS)/

xrp_linux_native.o: xrp-linux-native/xrp_linux_native.c $(XRP_INCS)/xrp_api.h $(XRP_INCS)/xrp_kernel_defs.h
	$(CC) -c $(CFLAGS) -o $@ $< -I$(XRP_INCS)/

libxrp.a: xrp_linux_native.o
	$(AR) rcs $@ $<


clean:
	rm -rf $(APP) *.o

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $< -Iinclude

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $< -Iinclude
