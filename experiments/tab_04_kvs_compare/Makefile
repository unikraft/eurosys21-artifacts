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
	$(WORKDIR)/build.sh

.PHONY: run
run:
	$(WORKDIR)/udp_echo.sh

.PHONY: plot
plot:
	@-

.PHONY: clean
clean:
	$(WORKDIR)/clean.sh
