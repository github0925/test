CFLAGS +=-I. -g -O0 -Wall -fpermissive
LDFLAGS += -lstdc++ -lpthread -L.
INCLUDE += \
	-I ./include \
	-I${STAGING_INCDIR} \
	-I${STAGING_INCDIR}/drm

TARGETS := example01 example02 example03 example04 example05


all:clean $(TARGETS)
	$(info build all)

version:
	date > version.h

example01: example01.o DrmDisplay.o
	$(CXX) $^ $(INCLUDE) -ldrm $(LDFLAGS) -o $@

example02: example02.o DrmDisplay.o
	$(CXX) $^ $(INCLUDE) -ldrm $(LDFLAGS) -o $@

example03: example03.o DrmDisplay.o
	$(CXX) $^ $(INCLUDE) -ldrm $(LDFLAGS) -o $@

example04: example04.o DrmDisplay.o
	$(CXX) $^ $(INCLUDE) -ldrm -lEGL -lGLES_CM  $(LDFLAGS) -o $@

example05: example05.o DrmDisplay.o
	$(CXX) $^ $(INCLUDE) -ldrm -lEGL -lGLES_CM  $(LDFLAGS) -o $@

%.o:%.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $^

.PHONY: clean
clean:
	-rm -f *.o $(TARGETS)
