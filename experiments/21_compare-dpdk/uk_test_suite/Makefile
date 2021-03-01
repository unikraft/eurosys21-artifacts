UK_ROOT ?= $(PWD)/../aux/unikraft
UK_LIBS ?= $(PWD)/../aux/libs/
UK_PLATS ?= $(PWD)/../../plats
empty:=
SPACE:=$(empty) $(empty)
LIBS_NAME ?= pthread newlib intel-intrinsics uk-dpdk 
LIBS := $(subst $(SPACE),:,$(addprefix $(UK_LIBS)/,$(LIBS_NAME)))
#PLAT_NAME ?= container
#PLATS := $(subst $(SPACE),:,$(addprefix $(UK_PLATS)/,$(PLAT_NAME)))

$(info $(LIBS))


all:
	@$(MAKE) -j1 -C $(UK_ROOT) A=$(PWD) L="$(LIBS)" P="$(PLATS)" fetch
	@$(MAKE) -j1 -C $(UK_ROOT) A=$(PWD) L="$(LIBS)" P="$(PLATS)" prepare
	@$(MAKE) -C $(UK_ROOT) A=$(PWD) L="$(LIBS)" P="$(PLATS)"

$(MAKECMDGOALS):
	@$(MAKE) -C $(UK_ROOT) A=$(PWD) L="$(LIBS)" P="$(PLATS)" $(MAKECMDGOALS)
