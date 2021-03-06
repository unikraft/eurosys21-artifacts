#
# Files and directories
#
WORKDIR ?= $(CURDIR)
RESULTS ?= $(WORKDIR)/results/merged.csv
PLOT_FORMAT ?= svg
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

.PHONY: clean
clean:
	$(WORKDIR)/clean.sh

.PHONY: plot
plot:
	$(SNAKE) $(WORKDIR)/plot.py --data $(RESULTS) --output $(PLOT)

.PHONY: prepare
prepare:
	$(WORKDIR)/setup.sh

.PHONY: run
run:
	$(WORKDIR)/run_all.sh
	$(WORKDIR)/merge.sh
