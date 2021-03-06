#
# Files and directories
#
WORKDIR ?= $(CURDIR)
RESULTS ?= $(WORKDIR)/results
PLOT ?= $(WORKDIR)/$(notdir $(WORKDIR)).svg

#
# Tools
#
SNAKE ?= python3

#
# Targets
#
.PHONY: all
all: prepare run plot

.PHONY: prepare
prepare:
	$(WORKDIR)/setup.sh

.PHONY: run
run:
	$(WORKDIR)/run_unikraft.sh
	$(WORKDIR)/run_vm_linux.sh

.PHONY: plot
plot:
	$(WORKDIR)/plot.sh
	$(SNAKE) $(WORKDIR)/plot.py --data $(RESULTS) --output $(PLOT)

.PHONY: clean
clean:
	$(WORKDIR)/clean.sh

.PHONY: all clean plot prepare run
