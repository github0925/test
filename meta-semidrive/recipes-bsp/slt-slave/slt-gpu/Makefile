
OBJS = slt_gpu.o \
       gles3test1/gles3test1.o \
       gles3test1/maths.o \
       gles3test1/eglutils.o \
       ocl_unit_test/ocltest.o \
       ocl_unit_test/ocltestmetrics.o

LIB = slt_gpu.so

LDLIBS = -lGLESv2 \
         -lOpenCL \
         -lPVROCL

all: $(LIB)

$(LIB): $(OBJS)
	$(CXX) $(LDFLAGS) $(LDLIBS) -shared -o $(LIB) $(OBJS)

clean:
	rm -f $(OBJS)

%.o: %.c
	$(CC) -fpic -c $(CFLAGS) -o $@ $<

