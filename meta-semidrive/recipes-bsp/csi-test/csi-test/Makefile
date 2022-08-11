CFLAGS +=-I. -g -O0 -Wall -fpermissive
LDFLAGS += -lstdc++ -lpthread -L.
INCLUDE := \
	-I ./include \
	-I${STAGING_INCDIR} \
	-I${STAGING_INCDIR}/drm

TARGETS = csi-test csi-test-gl
APP_OBJS = csitest.o csitest_gl.o

# Add any other object files to this list below

all: $(TARGETS)

csi-test: csitest.o
	$(CXX) $^ $(INCLUDE) -ldrm -lpthread $(LDFLAGS) -o $@

csi-test-gl: csitest_gl.o
	$(CXX) $^ $(INCLUDE) -ldrm -lEGL -lGLESv2 $(LDFLAGS) -o $@

%.o:%.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $^

.PHONY: clean
clean:
	-rm -f *.o $(TARGETS)
