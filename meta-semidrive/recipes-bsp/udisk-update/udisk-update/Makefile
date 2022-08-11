# Partition parse library
CC1 = /tool/gcc_linaro/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc-7.3.1
CFLAGS	= -Wall -O -fpic
INC	= include
INCLUDEFLAGS = --include-directory=$(INC)


OBJS2	=	storage_device.o \
		crc32.o \
		partition_parser.o \
		ab_partition_parser.o \
		slots_parse.o \
		storage_dev_ospi.o \
		pac_update.o \
		sparse_parser.o \
		update.o

#LIB	= libslots_parse.so

SAMPLE_NEW	= udisk_update

TARGETS	=   $(SAMPLE_NEW)

%.o:%.c
	 @$(CC1) -o $@ -c $< $(CFLAGS) $(INCLUDEFLAGS)

%.d:%.c
	@set -e; rm -f $@; $(CC1) -MM $< $(INCLUDEFLAGS) > $@.$$$$; \
	sed 's,\($*\)\.o[ : ]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(SAMPLE_NEW):$(OBJS2)
	$(CC1) -o $@ $^ $(LDFLAGS) -static
	echo "output: $@"

-include $(OBJS2:.o=.d)


.PHONY:all
all	: $(TARGETS)

.PHONY:clean
clean:
	@rm -f  $(TARGETS) *.o *.d *.d.*
