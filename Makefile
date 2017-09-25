TOPDIR          =.

exclude_dirs    = include  bin  lib

#dirs:=$(shell find . -maxdepth 1 -type d)
dirs= \
	src/modules/apptest/capture_glrender_main


dirs:=$(basename $(patsubst ./%,%,$(dirs)))
dirs:=$(filter-out $(exclude_dirs),$(dirs))
subdirs:=$(dirs)

SUBDIRS:$(subdirs)
	for dir in $(subdirs);\
	do make debug="$(debug)" -C $$dir all||exit 1;\
	done

include $(TOPDIR)/makefile.env
