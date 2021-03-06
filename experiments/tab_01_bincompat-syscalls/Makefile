#
# Files and directories
#
WORKDIR ?= $(CURDIR)
RESULTS ?= $(WORKDIR)/results/table.txt
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

.PHONY: prepare
prepare:
	$(WORKDIR)/clone-deps.sh "$(WORKDIR)"
	$(WORKDIR)/build-elfloader.sh "$(WORKDIR)"
	$(WORKDIR)/build-benchmark.sh "$(WORKDIR)"
	$(WORKDIR)/build-fses.sh "$(WORKDIR)"

.PHONY: run
run:
	$(WORKDIR)/run.sh "$(WORKDIR)"
	$(WORKDIR)/parse.sh "$(WORKDIR)"
	$(WORKDIR)/process.sh "$(WORKDIR)"

.PHONY: plot
plot:
	$(SNAKE) $(WORKDIR)/plot.py --data $(RESULTS) --output $(PLOT)

.PHONY: clean
clean:
	$(WORKDIR)/clean.sh "$(WORKDIR)"

.PHONY: properclean
properclean: clean
	@rm -rvf "${WORKDIR}/results/"*
