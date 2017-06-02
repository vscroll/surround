SUBDIRS = \
        src/modules/apptest/capture_main \
        src/modules/apptest/panoimage_main \
        src/modules/apptest/render_main \
        src/modules/apptest/calibrate_main \
        src/modules/apptest/controller_main \
        src/modules/apptest/v4l2_main \
        src/modules/apptest/ipu_main \
        src/modules/apptest/g2d_main \
        src/modules/apptest/fb_main \
        src/modules/apptest/colorconvert_main

define make_subdir
	@for subdir in $1 ; do \
		( cd $$subdir && make) \
	done;
endef

surroundpark:
	$(call make_subdir, $(SUBDIRS))

