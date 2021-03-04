#
# Files and directories
#
WORKDIR ?= $(CURDIR)

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
	@-

.PHONY: plot
plot:
	@echo "Not possible."

.PHONY: clean
clean:
	@-

.PHONY: properclean
properclean:
	@-
