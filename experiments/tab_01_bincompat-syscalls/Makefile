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
	@./clone-deps.sh "$(WORKDIR)"
	@./build-elfloader.sh "$(WORKDIR)"

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
