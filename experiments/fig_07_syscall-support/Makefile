#
# Files and directories
#
WORKDIR ?= $(CURDIR)
PLOT ?= $(WORKDIR)/$(notdir $(WORKDIR)).svg

all:
	$(WORKDIR)/setup.sh
	$(WORKDIR)/run_all.sh
	$(WORKDIR)/plot.sh

clean:
	$(WORKDIR)/clean.sh

plot:
	$(WORKDIR)/plot.sh
	$(CP) $(WORKDIR)/fig_07_syscall-support.svg $(PLOT)

prepare:
	$(WORKDIR)/setup.sh

run:
	$(WORKDIR)/run_all.sh

.PHONY:	all clean plot prepare run
