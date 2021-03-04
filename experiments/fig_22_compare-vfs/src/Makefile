UK_ROOT ?= $(PWD)/../../../unikraft
UK_LIBS ?= $(PWD)/../../../libs

WITH_TLSF       ?= n

LIBS-y := $(UK_LIBS)/shfs
LIBS-$(WITH_TLSF) := $(LIBS-y):$(UK_LIBS)/tlsf

APP := $(PWD)

all:
	@$(MAKE) -C $(UK_ROOT) L=$(LIBS-y) A=$(APP) O=$(PWD)/build C=$(PWD)/.config -j4 fetch && \
	$(MAKE) -C $(UK_ROOT) L=$(LIBS-y) A=$(APP) O=$(PWD)/build C=$(PWD)/.config -j1 prepare && \
	$(MAKE) -C $(UK_ROOT) L=$(LIBS-y) A=$(APP) O=$(PWD)/build C=$(PWD)/.config

UK_NOBUILD := menuconfig kmenuconfig guiconfig nconfig xconfig gconfig oldconfig syncconfig scriptsyncconfig olddefconfig randconfig defconfig savedefconfig allyesconfig allnoconfig help properclean distclean print-version print-libs print-lds print-objs print-srcs print-vars

ifeq (,$(filter $(MAKECMDGOALS),$(UK_NOBUILD)))
$(MAKECMDGOALS):
	@$(MAKE) -C $(UK_ROOT) L=$(LIBS-y) A=$(APP) O=$(PWD)/build C=$(PWD)/.config -j4 fetch && \
	$(MAKE) -C $(UK_ROOT) L=$(LIBS-y) A=$(APP) O=$(PWD)/build C=$(PWD)/.config -j1 prepare && \
	$(MAKE) -C $(UK_ROOT) L=$(LIBS-y) A=$(APP) O=$(PWD)/build C=$(PWD)/.config $(MAKECMDGOALS)
else
$(MAKECMDGOALS):
	@$(MAKE) -C $(UK_ROOT) L=$(LIBS-y) A=$(APP) -j1 O=$(PWD)/build C=$(PWD)/.config $(MAKECMDGOALS)
endif

%.o: %.c
	$(CC) $(LINUX_CFLAGS) -c $< -o $@

LINUX_CFLAGS+=-Wall -Wextra -O2 -g3 -fpie
LINUX_LDFLAGS+=-static-pie

main: main.c
	$(CC) $(LINUX_LDFLAGS) $^ $(LINUX_LDLIBS) -o $@

linux: main

linux-clean:
	$(RM) main *.o
