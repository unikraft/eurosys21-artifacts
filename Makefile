# SPDX-License-Identifier: BSD-3-Clause
# Authors: Alexander Jung <a.jung@lancs.ac.uk>
WORKDIR ?= $(CURDIR)
IMAGE ?= unikraft/eurosys21-artifacts-plot:latest

DOCKER ?= docker

.PHONY: docker
docker: docker-plots

.PHONY: docker-plots
docker-plots:
docker-plots:
	$(DOCKER) build \
		-f $(WORKDIR)/support/Dockerfile.plots \
		--tag $(IMAGE) \
		--cache-from $(IMAGE) \
		.
