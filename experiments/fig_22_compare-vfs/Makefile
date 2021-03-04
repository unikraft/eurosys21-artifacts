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
	@./clone-deps.sh "$(WORKDIR)"
	@./build-shfstools.sh "$(WORKDIR)"
	@./build-benchmark.sh "$(WORKDIR)"
	@./build-fses.sh "$(WORKDIR)"

.PHONY: run
run:
	@-

.PHONY: plot
plot:
	$(SNAKE) $(WORKDIR)/plot.py --data $(RESULTS) --output $(PLOT)

.PHONY: clean
clean:
	@-

.PHONY: properclean
properclean:
	@-
