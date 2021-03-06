#
# Files and directories
#
WORKDIR ?= $(CURDIR)

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

.PHONY: plot
plot:
	$(WORKDIR)/process.sh "$(WORKDIR)"
	$(SNAKE) $(WORKDIR)/plot.py

.PHONY: clean
clean:
	$(WORKDIR)/clean.sh "$(WORKDIR)"

.PHONY: properclean
properclean: clean
	@rm -rvf "${WORKDIR}/results/"*
