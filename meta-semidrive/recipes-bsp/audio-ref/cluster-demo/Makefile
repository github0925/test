APP := cluster_audio_demo

CLUSTER_AUDIO_OBJ := \
	src/cluster_audio.o

CLUSTER_AUDIO_LIBS := \
	-lpthread -lasound

CFLAGS += -Wall -Werror

all: $(APP)

cluster_audio: $(CLUSTER_AUDIO_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)  $(CLUSTER_AUDIO_LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

clean:
	rm -rf $(APP) src/*.o
