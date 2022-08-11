CFLAGS :=-g -O0 -Wall
CPPFLAGS := -g -O0 -Wall -Wno-reorder  -std=c++11 -fpermissive -fPIC
LDFLAGS = -lstdc++ -lpthread -fPIC \
	-L . \
	-L $(SDKTARGETSYSROOT)/usr/lib
INCLUDE += \
		   -I . \
		   -I ./include \
		   -I ./display \
		   -I ./allocator \
		   -I tests \
		   -I${SDKTARGETSYSROOT}/usr/include/drm \
		   -I${STAGING_KERNEL_DIR}/include/uapi/drm \

TARGETS := libdrmdisplay.so libhwconverter.so test_hwconverter

all:clean $(TARGETS)
	$(info build all)

version:
	date > version.h

test_hwconverter:  test_hwconvert.o
	$(warning "SDKTARGETSYSROOT = ${SDKTARGETSYSROOT}")
	$(info "pwd: $(shell pwd)")
	$(CXX) $(LDFLAGS) -ldrm -lminigbm -ldrmdisplay -lhwconverter  $^ -o $@

libhwconverter.so: buildConverter.o HwConverter.o G2dConverter.o GbmAllocator.o DrmAllocator.o
	$(CXX) -shared -fPIC -ldrm -lminigbm -ldrmdisplay $(LDFLAGS)  $^ -o $@

libdrmdisplay.so: DrmDisplay.o
	$(warning "STAGING_INCDIR = ${STAGING_INCDIR}")
	$(info "STAGING_KERNEL_DIR = ${STAGING_KERNEL_DIR}")
	$(CXX) -shared -fPIC -ldrm $(LDFLAGS) -o $@ $^

%.o:%.c
	$(CXX) $(CFLAGS) $(INCLUDE) -c $^

%.o:%.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDE) -c $^

%.o:display/%.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDE) -c $^

%.o:tests/%.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDE) -c $^

%.o:allocator/%.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDE) -c $^

.PHONY: clean install
clean:
	-rm -f *.o $(TARGETS)
	-rm -rf install


install:
	mkdir -p install/lib install/include
	cp libs/yocto/gbm/* install/lib/
	cp lib*.so install/lib/
	cp include/*.h install/include/
	cp -r tests install/
