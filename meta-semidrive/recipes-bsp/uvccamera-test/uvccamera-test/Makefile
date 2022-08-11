
APP = uvccamera-test
APP_OBJS = uvccapture.o v4l2uvc.o

# Add any other object files to this list below

all: $(APP)

$(APP): $(APP_OBJS)
	$(CC) $(LDFLAGS) $(XPM_LIB) $(MATH_LIB) -o $(APP) $(APP_OBJS) $(LDLIBS) -ljpeg

clean:
	rm -rf $(APP) *.o

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

