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
	@-

.PHONY: run
run:
	$(WORKDIR)/run.sh

.PHONY: plot
plot:
	$(SNAKE) $(WORKDIR)/plot.py --data $(RESULTS) --output $(PLOT)

.PHONY: clean
clean:
	$(RM) -rf cscope.*
	$(RM) -rf *.symbols
	$(RM) -rf *.deps
	$(RM) -rf unikraft/
	$(RM) -rf tmp
	$(RM) -rf output/

.PHONY: properclean
properclean:
	@-
