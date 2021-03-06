#
# Directories
#
_MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
TOOLSDIR := $(patsubst %/,%,$(dir $(_MKFILE_PATH)))
BUILDDIR ?= $(TOOLSDIR)/../build
PATCHDIR ?= $(TOOLSDIR)/patches

#
# Tools
#
GIT ?= git
CP ?= cp

#
# Targets
#

.PHONY: all
all: wrk solo5-hvt-noop-timer

.PHONY: wrk

WRK_VERSION ?= a211dd5a7050b1f9e8a9870b95513060e72ac4a0

wrk: $(BUILDDIR)/wrk
	$(MAKE) -C $(BUILDDIR)/$@
	$(CP) $(BUILDDIR)/$@/wrk $(CURDIR)

$(BUILDDIR)/wrk:
	$(GIT) clone https://github.com/wg/wrk.git $(BUILDDIR)/$@
	(cd $(BUILDDIR)/$@; $(GIT) checkout $(WRK_VERSION))

.PHONY: solo5-hvt-noop-timer

SOLO5_VERSION ?= 0eb8cb8f57943e7872a94e19695911a1e0b8aef6

solo5-hvt-noop-timer: $(BUILDDIR)/solo5-hvt-noop-timer
	(cd $(BUILDDIR)/$@; $(GIT) checkout $(SOLO5_VERSION))
	(cd $(BUILDDIR)/$@; $(GIT) am < $(PATCHDIR)/$@.patch)
	(cd $(BUILDDIR)/$@; ./configure.sh)
	$(MAKE) -C $(BUILDDIR)/$@
	$(CP) $(BUILDDIR)/$@/tenders/hvt/solo5-hvt $(TOOLSDIR)/$@

$(BUILDDIR)/solo5-hvt-noop-timer:
	$(GIT) clone https://github.com/solo5/solo5.git $@


.PHONY: solo5-hvt

solo5-hvt: $(BUILDDIR)/solo5-hvt
	(cd $(BUILDDIR)/$@; $(GIT) checkout $(SOLO5_VERSION))
	(cd $(BUILDDIR)/$@; ./configure.sh)
	$(MAKE) -C $(BUILDDIR)/$@
	$(CP) $(BUILDDIR)/$@/tenders/hvt/solo5-hvt $(TOOLSDIR)/$@

$(BUILDDIR)/solo5-hvt:
	$(GIT) clone https://github.com/solo5/solo5.git $@
