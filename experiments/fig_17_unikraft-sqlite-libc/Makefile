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
	$(WORKDIR)/sqlite_experiments.sh --prepare

.PHONY: run
run:
	$(WORKDIR)/sqlite_experiments.sh --run

.PHONY: plot
plot:
	PYTHONPATH=$(WORKDIR)/../common $(SNAKE) $(WORKDIR)/plot.py $(RESULTS) $(PLOT)

.PHONY: clean
clean:
	$(WORKDIR)/sqlite_experiments.sh --clean

.PHONY: properclean
properclean: clean
