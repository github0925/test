
APP = semilink-test

APP_OBJS := src/main.o \
            src/SemidriveLink.o \
            src/SemidriveLinkThread.o \
            src/Message512.o \
            src/Message513.o \
            src/Message514.o \
            src/Message521.o

# Add any other object files to this list below

all: $(APP)

$(APP): $(APP_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ -pthread

clean:
	rm -rf $(APP) *.o

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $< -Iinc

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $< -Iinc
