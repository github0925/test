# Partition parse library
CFLAGS	= -Wall -O -fpic
INC	= include
INCLUDEFLAGS = --include-directory=$(INC)
OBJS	=	storage_device.o \
		crc32.o \
		partition_parser.o \
		ab_partition_parser.o \
		slots_parse.o \
		storage_dev_ospi.o

LIB	= libslots_parse.so
SAMPLE	= sample
SAMPLE_MARK_SUCCESS	= sample_mark_success

TARGETS	= $(LIB) $(SAMPLE) $(SAMPLE_MARK_SUCCESS)

$(LIB):$(OBJS)
	@$(CC) -fpic -shared -o $@ $^ $(LDFLAGS)

%.o:%.c
	@$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDEFLAGS)

%.d:%.c
	@set -e; rm -f $@; $(CC) -MM $< $(INCLUDEFLAGS) > $@.$$$$; \
	sed 's,\($*\)\.o[ : ]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(SAMPLE):sample.c
	@$(CC) -o $@ $^ $(INCLUDEFLAGS) $(LIB)

$(SAMPLE_MARK_SUCCESS):sample_mark_success.c
	@$(CC) -o $@ $^ $(INCLUDEFLAGS) $(LIB)

-include $(OBJS:.o=.d)

.PHONY:all
all	: $(TARGETS)

.PHONY:clean
clean:
	@rm -f $(LIB) $(SAMPLE) $(SAMPLE) $(SAMPLE_MARK_SUCCESS) *.o *.d *.d.*
