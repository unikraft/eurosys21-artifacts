# SPDX-License-Identifier: BSD-3-Clause
# Authors: Alexander Jung <a.jung@lancs.ac.uk>
WORKDIR ?= $(CURDIR)

DOCKER ?= docker
DOCKER_FORCE_BUILD ?= n
IMAGES ?= $(subst Dockerfile.,,$(notdir $(wildcard $(WORKDIR)/support/Dockerfile.*)))
IMAGE_PREFFIX ?= unikraft/eurosys21-artifacts-

#
# Targets
#

.PHONY: all
all: docker

.PHONY: docker
docker: $(addprefix docker-,$(IMAGES))
docker-%:
	$(DOCKER) pull $(IMAGE_PREFFIX)$*:latest || true
ifeq ($(DOCKER_FORCE_BUILD),y)
	$(DOCKER) build \
		-f $(WORKDIR)/support/Dockerfile.$* \
		--tag $(IMAGE_PREFFIX)$*:latest \
		--cache-from $(IMAGE_PREFFIX)$*:latest \
		$(DOCKER_BUILD_EXTRA) \
		.
endif

include $(WORKDIR)/tools/Makefile
