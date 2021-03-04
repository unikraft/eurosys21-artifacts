# SPDX-License-Identifier: BSD-3-Clause
# Authors: Alexander Jung <a.jung@lancs.ac.uk>

WORKDIR ?= $(CURDIR)
IMAGESDIR ?= $(WORKDIR)/images
CONFIGSDIR ?= $(WORKDIR)/configs
RESULTS ?= $(WORKDIR)/results.csv
PLOT ?= $(WORKDIR)/$(notdir $(WORKDIR)).svg

KRAFT_IMAGE ?= unikraft/eurosys21-artifacts-kraft:latest

IMAGES ?= $(sort $(notdir $(wildcard $(CONFIGSDIR)/*)))

RM ?= rm
SNAKE ?= python3
DOCKER ?= docker
DRUN ?= $(DOCKER) run -it --rm \
        -v $(1):/usr/src/unikraft/apps/app \
				-w /usr/src/unikraft/apps/app $(KRAFT_IMAGE) $(2)

ARCH ?= x86_64
PLAT ?= kvm

.PHONY: all
all: prepare run plot

.PHONY: prepare
prepare: helloworld nginx redis sqlite

.PHONY: helloworld
helloworld: helloworld_default helloworld_dce helloworld_lto helloworld_dce_lto
helloworld_%:
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v list pull -w .unikraft unikraft@0.4)
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v configure -m $(ARCH) -p $(PLAT))
	$(call DRUN,$(CONFIGSDIR)/$@,kraft build)
	cp $(CONFIGSDIR)/$@/build/app_$(PLAT)-$(ARCH) $(IMAGESDIR)/$@_$(PLAT)-$(ARCH)

.PHONY: nginx
nginx: nginx_default nginx_dce nginx_lto nginx_dce_lto
nginx_%:
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v list pull -w .unikraft unikraft@0.4)
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v list pull -w .unikraft lib/pthread-embedded@0.4)
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v list pull -w .unikraft lib/newlib@0.4)
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v list pull -w .unikraft lib/pcre@5f7140e)
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v list pull -w .unikraft lib/lwip@0.4)
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v list pull -w .unikraft lib/openssl@0.4)
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v list pull -w .unikraft lib/nginx@0.4)
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v configure -m $(ARCH) -p $(PLAT))
	$(call DRUN,$(CONFIGSDIR)/$@,kraft build)
	cp $(CONFIGSDIR)/$@/build/app_$(PLAT)-$(ARCH) $(IMAGESDIR)/$@_$(PLAT)-$(ARCH)

.PHONY: redis
redis: redis_default redis_dce redis_lto redis_dce_lto
redis_%:
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v list pull -w .unikraft unikraft@0.4)
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v list pull -w .unikraft lib/pthread-embedded@0.4)
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v list pull -w .unikraft lib/newlib@0.4)
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v list pull -w .unikraft lib/lwip@0.4)
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v list pull -w .unikraft lib/redis@0.4)
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v list pull -w .unikraft redis@0.4)
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v configure -m $(ARCH) -p $(PLAT))
	$(call DRUN,$(CONFIGSDIR)/$@,kraft build)
	cp $(CONFIGSDIR)/$@/build/app_$(PLAT)-$(ARCH) $(IMAGESDIR)/$@_$(PLAT)-$(ARCH)

.PHONY: sqlite
sqlite: sqlite_default sqlite_dce sqlite_lto sqlite_dce_lto
sqlite_%:
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v list pull -w .unikraft unikraft@0.4)
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v list pull -w .unikraft lib/pthread-embedded@0.4)
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v list pull -w .unikraft lib/newlib@0.4)
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v list pull -w .unikraft lib/sqlite@0.4)
	$(call DRUN,$(CONFIGSDIR)/$@,kraft -v configure -m $(ARCH) -p $(PLAT))
	$(call DRUN,$(CONFIGSDIR)/$@,kraft build)
	cp $(CONFIGSDIR)/$@/build/app_$(PLAT)-$(ARCH) $(IMAGESDIR)/$@

.PHONY: run
run: $(addprefix checksize-,$(IMAGES))
	@echo "Results saved to to $(RESULTS)"
checksize-%:
	@du -b $(IMAGESDIR)/$* | awk '{ print "$*\t" $$1 }' >> $(RESULTS)

.PHONY: plot
plot:
	PYTHONPATH=$(WORKDIR)/../common $(SNAKE) $(WORKDIR)/plot.py --data $(RESULTS) --output $(PLOT)

.PHONY: clean
clean: $(addprefix clean-,$(IMAGES))

clean-%:
	$(RM) -rf $(CONFIGSDIR)/$*/build
	$(RM) -rf $(CONFIGSDIR)/$*/.unikraft
	$(RM) -rf $(CONFIGSDIR)/$*/.config

.PHONY: properclean
properclean: clean
	$(DOCKER) rmi $(KRAFT_IMAGE)
