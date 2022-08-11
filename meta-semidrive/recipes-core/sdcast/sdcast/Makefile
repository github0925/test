INCLUDE := -I ./include
INCLUDE += -I/usr/include/libdrm

# LIBDRM_CFLAGS = $(shell pkg-config --cflags libdrm)
# LIBDRM_LIBS = $(shell pkg-config --libs libdrm)

CFLAGS = -g -O -Wall -fPIC  -Werror -Wno-poison-system-directories \
		-Wno-unused-variable

LDFLAGS = -fPIC -ldrm -lpthread

TARGET := libSdCast.so sdcast_test

SRCS = $(wildcard src/*.cpp)
OBJS := $(SRCS:%.cpp=%.o)

all: $(TARGET)
	$(info build all)

sdcast_test: test/main.o libSdCast.so
	$(CXX) $^ $(INCLUDE) $(LDFLAGS) -o $@

test/main.o:test/main.cpp
	$(CXX) -c $(CFLAGS) -lSdCast -o $@ $^ $(INCLUDE)

libSdCast.so: $(OBJS)
	$(CXX) -shared $(LDFLAGS) -o $@ $(OBJS)

%.o:%.cpp
	$(CXX) -c $(CFLAGS) -o $@ $^ $(INCLUDE)

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)
