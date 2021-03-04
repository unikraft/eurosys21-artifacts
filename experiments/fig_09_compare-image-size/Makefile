#
# Files and directories
#
WORKDIR ?= $(CURDIR)
IMPLDIR ?= $(WORKDIR)/impl
RESULTSDIR ?= $(WORKDIR)/results
PLOT ?= $(WORKDIR)/$(notdir $(WORKDIR)).svg

#
# Arguments
#
NGINX_VERSION ?= 1.15.6
REDIS_VERSION ?= 5.0.6

#
# Tools
#
SNAKE ?= python3
DOCKER ?= docker

#
# Targets
#
.PHONY: all
all: prepare run plot

.PHONY: prepare
prepare:
	$(DOCKER) pull olivierpierre/hermitux
	$(DOCKER) pull nginx:$(NGINX_VERSION)-alpine
	$(DOCKER) pull redis:$(REDIS_VERSION)-alpine
	$(DOCKER) pull hlefeuvre/alpine-sqlite
	$(DOCKER) pull hlefeuvre/rump
	$(DOCKER) pull hlefeuvre/osv

.PHONY: run
run:
	$(IMPLDIR)/alpine-size.sh

.PHONY: plot
plot:
	$(SNAKE) $(WORKDIR)/plot.py --data $(RESULTS) --output $(PLOT)

.PHONY: clean
clean:
	$(RM) $(ALPINE_IMAGE)

.PHONY: properclean
properclean:
	$(DOCKER) rmi olivierpierre/hermitux
	$(DOCKER) rmi nginx:$(NGINX_VERSION)-alpine
	$(DOCKER) rmi redis:$(REDIS_VERSION)-alpine
	$(DOCKER) rmi hlefeuvre/alpine-sqlite
	$(DOCKER) rmi hlefeuvre/rump
	$(DOCKER) rmi hlefeuvre/osv
