
ARCH_CFLAGS += -O0 -g -Wno-format-security -Werror -std=c99
# Undef INT128 so host behavior as same with 32bit machine for BearSSL
ARCH_CFLAGS += -U__SIZEOF_INT128__
ifdef COV
CFG_CFLAGS += -fprofile-arcs -ftest-coverage
endif

LDFLAGS += -lc -lm
ifdef COV
LDFLAGS += -lgcov --coverage
endif
CFG_CFLAGS += -DARCH_PTR_64BIT

CINCLUDE += -I$(TOPDIR)/crypto/openssl/include/
LIBS += -L$(TOPDIR)/crypto/openssl/lib/ -pthread -lcrypto -lz -ldl -static-libgcc
