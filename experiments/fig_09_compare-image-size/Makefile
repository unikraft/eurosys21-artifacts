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
	$(IMPLDIR)/hermitux-light-size.sh
	$(IMPLDIR)/hermitux-size.sh
	$(IMPLDIR)/lupine-size.sh
	$(IMPLDIR)/mirage-size.sh
	$(IMPLDIR)/native-size.sh
	$(IMPLDIR)/osv-size.sh
	$(IMPLDIR)/rump-size.sh

.PHONY: plot
plot:
	$(SNAKE) $(WORKDIR)/plot.py --data $(RESULTS) --output $(PLOT)

.PHONY: clean
clean:
	$(RM) -rf $(IMPLDIR)/mirage-helloworld/_build
	$(RM) -rf $(IMPLDIR)/mirage-helloworld/.merlin
	$(RM) -rf $(IMPLDIR)/mirage-helloworld/.mirage.config
	$(RM) -rf $(IMPLDIR)/mirage-helloworld/dune
	$(RM) -rf $(IMPLDIR)/mirage-helloworld/dune-project
	$(RM) -rf $(IMPLDIR)/mirage-helloworld/dune.build
	$(RM) -rf $(IMPLDIR)/mirage-helloworld/dune.config
	$(RM) -rf $(IMPLDIR)/mirage-helloworld/key_gen.ml
	$(RM) -rf $(IMPLDIR)/mirage-helloworld/main.ml
	$(RM) -rf $(IMPLDIR)/mirage-helloworld/Makefile
	$(RM) -rf $(IMPLDIR)/mirage-helloworld/*.opam
	$(RM) -rf $(IMPLDIR)/mirage-helloworld/myocamlbuild.ml

.PHONY: properclean
properclean:
	$(DOCKER) rmi olivierpierre/hermitux
	$(DOCKER) rmi nginx:$(NGINX_VERSION)-alpine
	$(DOCKER) rmi redis:$(REDIS_VERSION)-alpine
	$(DOCKER) rmi hlefeuvre/alpine-sqlite
	$(DOCKER) rmi hlefeuvre/rump
	$(DOCKER) rmi hlefeuvre/osv
