CC = gcc
LD = $(CC)
CFLAGS+=-Wall -Wextra -O0 -g3 -fpie
LDFLAGS+=-static-pie
LDLIBS +=

-include .config
ifeq ($(CONFIG_SYSCALL_WARMUP),y)
CFLAGS += -DCONFIG_WARMUP=1
endif
ifeq ($(CONFIG_BENCHTSC),y)
CFLAGS += -DCONFIG_BENCHTSC=1
endif
ifeq ($(CONFIG_BENCHTSC_AND_LOOP),y)
CFLAGS += -DCONFIG_BENCHTSC_AND_LOOP=1
endif
ifneq ($(CONFIG_NB_ROUNDS),)
CFLAGS += -DCONFIG_NB_ROUNDS=$(CONFIG_NB_ROUNDS)
endif
ifneq ($(CONFIG_ITER_PER_ROUND),)
CFLAGS += -DCONFIG_ITER_PER_ROUND=$(CONFIG_ITER_PER_ROUND)
endif

all: syscallbench

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

syscallbench: bench.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	$(RM) syscallbench *.o

.PHONY: clean
