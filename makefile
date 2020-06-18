# example Makefile
#
#SHELL = /bin/sh
#INSTALL = ./bin
#INSTALL_PROGRAM = $(INSTALL)
#INSTALL_DATA = $(INSTALL) -m 644
#include Makefile.conf

DIRS = balancer calculator client
# the sets of directories to do various things in
BUILDDIRS = $(DIRS:%=build-%)
INSTALLDIRS = $(DIRS:%=install-%)
CLEANDIRS = $(DIRS:%=clean-%)
#TESTDIRS = $(DIRS:%=test-%)

all: $(BUILDDIRS)
$(DIRS): $(BUILDDIRS)
$(BUILDDIRS):
	$(MAKE) -C $(@:build-%=%)

install: $(INSTALLDIRS)
$(INSTALLDIRS):
	mkdir -p ./bin
	$(MAKE) -C $(@:install-%=%) install

#$(INSTALLDIRS):
#	$(MAKE) -C $(@:install-%=%) install

#test: $(TESTDIRS) all
#$(TESTDIRS):
#	$(MAKE) -C $(@:test-%=%) test

clean: $(CLEANDIRS)
$(CLEANDIRS):
	$(MAKE) -C $(@:clean-%=%) clean


.PHONY: subdirs $(DIRS)
.PHONY: subdirs $(BUILDDIRS)
.PHONY: subdirs $(INSTALLDIRS)
#.PHONY: subdirs $(TESTDIRS)
.PHONY: subdirs $(CLEANDIRS)
.PHONY: all install clean test
