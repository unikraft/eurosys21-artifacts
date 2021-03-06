#
# Files and directories
#
WORKDIR ?= $(CURDIR)
RESULTS ?= $(WORKDIR)
PLOT_FORMAT ?= svg
PLOT ?= $(WORKDIR)/$(notdir $(WORKDIR)).$(PLOT_FORMAT)
GZ ?= $(WORKDIR)/$(notdir $(WORKDIR)).gz

#
# Tools
#
SNAKE ?= python3
DOT ?= dot

#
# Targets
#
.PHONY: all
all: prepare run plot

.PHONY: prepare
prepare:
	$(WORKDIR)/prepare.sh

.PHONY: run
run:
	$(WORKDIR)/run.sh

.PHONY: plot
plot:
	$(SNAKE) $(WORKDIR)/plot.py --data $(RESULTS) --output $(GZ)
	$(DOT) -T$(PLOT_FORMAT) $(GZ) -o $(PLOT)

.PHONY: clean
clean:
	$(WORKDIR)/clean.sh
