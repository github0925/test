CC = gcc
AS = as
LD = gcc
OBJCOPY = objcopy

CFLAGS += -O0 -g

LDFLAGS += -lc

TGT = bin_injector

C_SRC ?= bin_injector.c

C_OBJ := $(C_SRC:.c=.o)
OBJS := $(C_OBJ)

.PHONY: clean

all: $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o $(TGT)

$(C_OBJ): %.o:%.c
	@echo "Compiling $<"
	$(AT)$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm *.o -rf
	-rm *.out -rf
