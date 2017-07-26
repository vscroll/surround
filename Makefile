TOPDIR          =.

exclude_dirs    = include  bin  lib

#dirs:=$(shell find . -maxdepth 1 -type d)
dirs= \
	src/tools/generate_cl_table \
	src/tools/update_crop_info \
	src/modules/apptest/alpha_main \
	src/modules/apptest/calibrate_main \
	src/modules/apptest/capture_main \
	src/modules/apptest/capture_pano_main \
	src/modules/apptest/colorconvert_main \
	src/modules/apptest/fb_main \
	src/modules/apptest/g2d_main \
	src/modules/apptest/ipu_main \
	src/modules/apptest/panoimage_main \
	src/modules/apptest/render_main \
	src/modules/apptest/v4l2_main

dirs:=$(basename $(patsubst ./%,%,$(dirs)))
dirs:=$(filter-out $(exclude_dirs),$(dirs))
subdirs:=$(dirs)

SUBDIRS:$(subdirs)
	for dir in $(subdirs);\
	do make debug="$(debug)" -C $$dir all||exit 1;\
	done

include $(TOPDIR)/makefile.env
