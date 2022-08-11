CC = gcc
AS = as
LD = gcc
OBJCOPY = objcopy

CFLAGS += -O0 -g

LDFLAGS += -lc

TARGET = ddr_seq_parser

C_SRC ?= src/ddr_seq_parser.c src/DDR_regs.c src/crc32.c

C_OBJ := $(C_SRC:.c=.o)
OBJS := $(C_OBJ)

.PHONY: clean

all: $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o $(TARGET)

$(C_OBJ): %.o:%.c
	@echo "Compiling $<"
	$(AT)$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm *.o -rf
	-rm *.out -rf
