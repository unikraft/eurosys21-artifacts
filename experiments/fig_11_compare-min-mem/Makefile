#
# Files and directories
#
WORKDIR ?= $(CURDIR)
ROOTDIR ?= $(WORKDIR)/../..
LUPINEDIR ?= $(WORKDIR)/build/lupine
RESULTS ?= $(WORKDIR)/results
PLOT ?= $(WORKDIR)/$(notdir $(WORKDIR)).svg

#
# Tools
#
DOCKER ?= docker
SNAKE ?= python3

#
# Targets
#
.PHONY: all
all: prepare run plot

.PHONY: prepare
prepare:
	# $(ROOTDIR)/tools/build-lupine.sh -d $(LUPINEDIR)
	$(DOCKER) pull hlefeuvre/alpine-sqlite
	$(DOCKER) tag hlefeuvre/alpine-sqlite alpine-sqlite
	$(WORKDIR)/genimages.sh

.PHONY: run
run:
	$(WORKDIR)/benchmark.sh

.PHONY: plot
plot:
	$(SNAKE) $(WORKDIR)/plot.py --data $(RESULTS) --output $(PLOT)

.PHONY: clean
clean:
	@
