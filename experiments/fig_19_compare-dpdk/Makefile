#
# Files and directories
#
WORKDIR ?= $(CURDIR)
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
	$(info "This experiment cannot be called from run.sh.  Please read the README.md")

.PHONY: run
run:
	$(info "This experiment cannot be called from run.sh.  Please read the README.md")

.PHONY: plot
plot:
	$(SNAKE) $(WORKDIR)/plot.py --data $(RESULTS) --output $(PLOT)

.PHONY: clean
clean:
	$(WORKDIR)/clean.sh

.PHONY: properclean
properclean:
	@-
