CFLAGS=-g -O0 -Wall
CPPFLAGS = -g -O0 -Wall -Wno-reorder  -std=c++11 -fpermissive -fPIC

EXTERN_LIB_PATH:=..

LDFLAGS= -lstdc++ -lpthread -fPIC \
		 -L $(EXTERN_LIB_PATH)/lib \
		 -L $(SDKTARGETSYSROOT)/usr/lib
INCLUDE := \
	-I . \
	-I $(EXTERN_LIB_PATH)/include \
	-I $(SDKTARGETSYSROOT)/usr/include \
	-I $(SDKTARGETSYSROOT)/usr/include/drm \

TARGETS := test_hwconverter

all:clean $(TARGETS)
	$(info build all)

version:
	date > version.h

test_hwconverter:  test_hwconvert.o
	$(CXX) $(LDFLAGS) -ldrmdisplay -lhwconverter -ldrm -lminigbm -o $@ $^

%.o:%.c
	$(CXX) $(CFLAGS) $(INCLUDE) -c $^
%.o:%.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDE) -c $^

.PHONY: clean
clean:
	-rm -f *.o $(TARGETS)
