#
# Files and directories
#
WORKDIR ?= $(CURDIR)
RESULTS ?= $(WORKDIR)/results

#
# Targets
#
.PHONY: prepare
prepare:
	$(WORKDIR)/genimages.sh

.PHONY: run
run:
	$(WORKDIR)/benchmark.sh

.PHONY: plot
plot:
	@-

.PHONY: clean
clean:
	@-
