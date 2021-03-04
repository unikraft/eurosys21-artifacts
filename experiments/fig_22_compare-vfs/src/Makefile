UK_ROOT ?= $(PWD)/../../../unikraft
UK_LIBS ?= $(PWD)/../../../libs

WITH_TLSF       ?= y

LIBS-y := $(UK_LIBS)/shfs
LIBS-$(WITH_TLSF) := $(LIBS-y):$(UK_LIBS)/tlsf

APP := $(PWD)

UK_NOBUILD := menuconfig kmenuconfig guiconfig nconfig xconfig gconfig oldconfig syncconfig scriptsyncconfig olddefconfig randconfig defconfig savedefconfig allyesconfig allnoconfig help properclean distclean print-version print-libs print-lds print-objs print-srcs print-vars
LINUX_BUILD := fsbench-clean fsbench

all:
	@$(MAKE) -C $(UK_ROOT) L=$(LIBS-y) A=$(APP) O=$(PWD)/build C=$(PWD)/.config -j4 fetch && \
	$(MAKE) -C $(UK_ROOT) L=$(LIBS-y) A=$(APP) O=$(PWD)/build C=$(PWD)/.config -j1 prepare && \
	$(MAKE) -C $(UK_ROOT) L=$(LIBS-y) A=$(APP) O=$(PWD)/build C=$(PWD)/.config

ifeq (,$(filter $(MAKECMDGOALS),$(UK_NOBUILD) $(LINUX_BUILD)))
$(MAKECMDGOALS):
	@$(MAKE) -C $(UK_ROOT) L=$(LIBS-y) A=$(APP) O=$(PWD)/build C=$(PWD)/.config -j4 fetch && \
	$(MAKE) -C $(UK_ROOT) L=$(LIBS-y) A=$(APP) O=$(PWD)/build C=$(PWD)/.config -j1 prepare && \
	$(MAKE) -C $(UK_ROOT) L=$(LIBS-y) A=$(APP) O=$(PWD)/build C=$(PWD)/.config $(MAKECMDGOALS)
else
ifeq (,$(filter $(MAKECMDGOALS),$(LINUX_BUILD)))
$(MAKECMDGOALS):
	@$(MAKE) -C $(UK_ROOT) L=$(LIBS-y) A=$(APP) -j1 O=$(PWD)/build C=$(PWD)/.config $(MAKECMDGOALS)
endif
endif

LINUX_CFLAGS+=-Wall -Wextra -O2 -g3 -fpie
LINUX_LDFLAGS+=-static-pie

fsbench: main.c
	$(CC) $(LINUX_LDFLAGS) $^ $(LINUX_LDLIBS) -o $@

fsbench-clean:
	$(RM) fsbench *.o

.PHONY: $(UK_NOBUILD) fsbench-clean
