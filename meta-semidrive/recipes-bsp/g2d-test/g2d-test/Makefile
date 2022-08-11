CFLAGS +=-I. -g -O0 -Wall -fpermissive -DUSE_LINUX -D__YOCTO_G2D_TEST__
LDFLAGS += -lstdc++ -lpthread -L.
INCLUDE += \
	-I ./include \
	-I${STAGING_INCDIR} \
	-I${STAGING_INCDIR}/drm \
	-I${PWD}/../../../../../../source/linux/include/uapi/drm \

TARGETS := g2d_test

all:clean $(TARGETS)
	$(info build all)

version:
	date > version.h

g2d_test: main.o g2dapi.o drm_handle.o
	$(CXX) $^ $(INCLUDE) -ldrm $(LDFLAGS) -o $@

%.o:%.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $^

%.o:%.c
	$(CXX) $(CFLAGS) $(INCLUDE) -c $^


.PHONY: clean
clean:
	-rm -f *.o $(TARGETS)
