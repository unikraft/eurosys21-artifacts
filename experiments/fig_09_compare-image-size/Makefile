#
# Files and directories
#
WORKDIR ?= $(CURDIR)
IMPLDIR ?= $(WORKDIR)/impl
IMAGESDIR ?= $(WORKDIR)/images
RESULTSDIR ?= $(WORKDIR)/results
PLOT ?= $(WORKDIR)/$(notdir $(WORKDIR)).svg

#
# Arguments
#
NGINX_VERSION ?= 1.15.6
REDIS_VERSION ?= 5.0.6
KRAFT_IMAGE ?= unikraft/eurosys21-artifacts-kraft:latest
ARCH ?= x86_64
PLAT ?= kvm

#
# Tools
#
SNAKE ?= python3
DOCKER ?= docker
DRUN ?= $(DOCKER) run -it --rm \
        -v $(1):/usr/src/unikraft/apps/app \
				-w /usr/src/unikraft/apps/app $(KRAFT_IMAGE) $(2)

#
# Targets
#
.PHONY: all
all: prepare run plot

.PHONY: prepare
prepare: unikraft
	$(DOCKER) pull olivierpierre/hermitux
	$(DOCKER) pull nginx:$(NGINX_VERSION)-alpine
	$(DOCKER) pull redis:$(REDIS_VERSION)-alpine
	$(DOCKER) pull hlefeuvre/alpine-sqlite
	$(DOCKER) pull hlefeuvre/rump
	$(DOCKER) pull hlefeuvre/osv

.PHONY: unikraft
unikraft: unikraft-helloworld unikraft-nginx unikraft-redis unikraft-sqlite

.PHONY: unikraft-helloworld
unikraft-helloworld:
	$(call DRUN,$(IMPLDIR)/$@,kraft -v list pull -w .unikraft unikraft@0.4)
	$(call DRUN,$(IMPLDIR)/$@,kraft -v configure -m $(ARCH) -p $(PLAT) -F)
	$(call DRUN,$(IMPLDIR)/$@,kraft build)
	cp $(IMPLDIR)/$@/build/app_$(PLAT)-$(ARCH) $(IMAGESDIR)/$@

.PHONY: unikraft-nginx
unikraft-nginx:
	$(call DRUN,$(IMPLDIR)/$@,kraft -v list pull -w .unikraft unikraft@0.4)
	$(call DRUN,$(IMPLDIR)/$@,kraft -v list pull -w .unikraft lib/pthread-embedded@0.4)
	$(call DRUN,$(IMPLDIR)/$@,kraft -v list pull -w .unikraft lib/newlib@0.4)
	$(call DRUN,$(IMPLDIR)/$@,kraft -v list pull -w .unikraft lib/pcre@5f7140e)
	$(call DRUN,$(IMPLDIR)/$@,kraft -v list pull -w .unikraft lib/lwip@0.4)
	$(call DRUN,$(IMPLDIR)/$@,kraft -v list pull -w .unikraft lib/openssl@0.4)
	$(call DRUN,$(IMPLDIR)/$@,kraft -v list pull -w .unikraft lib/nginx@0.4)
	$(call DRUN,$(IMPLDIR)/$@,kraft -v configure -m $(ARCH) -p $(PLAT) -F)
	$(call DRUN,$(IMPLDIR)/$@,kraft build)
	cp $(IMPLDIR)/$@/build/app_$(PLAT)-$(ARCH) $(IMAGESDIR)/$@

.PHONY: unikraft-redis
unikraft-redis:
	$(call DRUN,$(IMPLDIR)/$@,kraft -v list pull -w .unikraft unikraft@0.4)
	$(call DRUN,$(IMPLDIR)/$@,kraft -v list pull -w .unikraft lib/pthread-embedded@0.4)
	$(call DRUN,$(IMPLDIR)/$@,kraft -v list pull -w .unikraft lib/newlib@0.4)
	$(call DRUN,$(IMPLDIR)/$@,kraft -v list pull -w .unikraft lib/lwip@0.4)
	$(call DRUN,$(IMPLDIR)/$@,kraft -v list pull -w .unikraft lib/redis@0.4)
	$(call DRUN,$(IMPLDIR)/$@,kraft -v list pull -w .unikraft redis@0.4)
	$(call DRUN,$(IMPLDIR)/$@,kraft -v configure -m $(ARCH) -p $(PLAT) -F)
	$(call DRUN,$(IMPLDIR)/$@,kraft build)
	cp $(IMPLDIR)/$@/build/app_$(PLAT)-$(ARCH) $(IMAGESDIR)/$@

.PHONY: unikraft-sqlite
unikraft-sqlite:
	$(call DRUN,$(IMPLDIR)/$@,kraft -v list pull -w .unikraft unikraft@0.4)
	$(call DRUN,$(IMPLDIR)/$@,kraft -v list pull -w .unikraft lib/pthread-embedded@0.4)
	$(call DRUN,$(IMPLDIR)/$@,kraft -v list pull -w .unikraft lib/newlib@0.4)
	$(call DRUN,$(IMPLDIR)/$@,kraft -v list pull -w .unikraft lib/sqlite@0.4)
	$(call DRUN,$(IMPLDIR)/$@,kraft -v configure -m $(ARCH) -p $(PLAT) -F)
	$(call DRUN,$(IMPLDIR)/$@,kraft build)
	cp $(IMPLDIR)/$@/build/app_$(PLAT)-$(ARCH) $(IMAGESDIR)/$@

.PHONY: run
run:
	$(IMPLDIR)/hermitux-light-size.sh
	$(IMPLDIR)/hermitux-size.sh
	$(IMPLDIR)/lupine-size.sh
	$(IMPLDIR)/mirage-size.sh
	$(IMPLDIR)/native-size.sh
	$(IMPLDIR)/osv-size.sh
	$(IMPLDIR)/rump-size.sh
	$(IMPLDIR)/unikraft-size.sh

.PHONY: plot
plot:
	PYTHONPATH=$(WORKDIR)/../common $(SNAKE) $(WORKDIR)/plot.py --data $(RESULTSDIR) --output $(PLOT)

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
	$(RM) -rf $(IMPLDIR)/unikraft-helloworld/build
	$(RM) -rf $(IMPLDIR)/unikraft-helloworld/.config
	$(RM) -rf $(IMPLDIR)/unikraft-helloworld/.unikraft
	$(RM) -rf $(IMPLDIR)/unikraft-nginx/build
	$(RM) -rf $(IMPLDIR)/unikraft-nginx/.config
	$(RM) -rf $(IMPLDIR)/unikraft-nginx/.unikraft
	$(RM) -rf $(IMPLDIR)/unikraft-redis/build
	$(RM) -rf $(IMPLDIR)/unikraft-redis/.config
	$(RM) -rf $(IMPLDIR)/unikraft-redis/.unikraft
	$(RM) -rf $(IMPLDIR)/unikraft-sqlite/build
	$(RM) -rf $(IMPLDIR)/unikraft-sqlite/.config
	$(RM) -rf $(IMPLDIR)/unikraft-sqlite/.unikraft

.PHONY: properclean
properclean:
	$(DOCKER) rmi olivierpierre/hermitux
	$(DOCKER) rmi nginx:$(NGINX_VERSION)-alpine
	$(DOCKER) rmi redis:$(REDIS_VERSION)-alpine
	$(DOCKER) rmi hlefeuvre/alpine-sqlite
	$(DOCKER) rmi hlefeuvre/rump
	$(DOCKER) rmi hlefeuvre/osv
