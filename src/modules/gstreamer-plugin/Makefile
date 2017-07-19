#before buildint it don't forget to export ROOTF_DIR and TOOLCHAIN (only if they are uncommented here)

# needs to source /opt/poky/x.x.x/environmentsetup.sh

TOPDIR                      = ../../..

TARGET                      = libgstgladas.so

LIBPATH                     = $(TOPDIR)/lib/
EXEPATH                     = $(TOPDIR)/bin/

TOOLCHAIN_ROOT              = /opt/fsl-imx-x11/3.14.52-1.1.0/sysroots
ROOTFS_DIR                  = $(HOME)/my-imx6/05_rootfs/distro-fsl-image
#TARGET_PATH_LIB 	        = $(ROOTFS_DIR)/usr/lib
#TARGET_PATH_INCLUDE         = $(ROOTFS_DIR)/usr/include
TARGET_PATH_LIB 	        = $(TOOLCHAIN_ROOT)/cortexa9hf-vfp-neon-poky-linux-gnueabi/usr/lib
TARGET_PATH_INCLUDE         = $(TOOLCHAIN_ROOT)/cortexa9hf-vfp-neon-poky-linux-gnueabi/usr/include

CFLAGS			            = -fPIC -DLINUX -DUSE_SOC_MX6 -Wall -O2 -fsigned-char -march=armv7-a -mfpu=neon -mthumb-interwork -mtune=cortex-a9 -mfpu=vfp -mfloat-abi=hard \
                                -DEGL_API_FB -DGPU_TYPE_VIV -DGL_GLEXT_PROTOTYPES -DENABLE_GPU_RENDER_20 \
			                    -I. -I$(TARGET_PATH_INCLUDE) \
                                -I$(TARGET_PATH_INCLUDE)/glib-2.0 \
                                -I$(TARGET_PATH_LIB)/glib-2.0/include \
                                -I$(TARGET_PATH_INCLUDE)/gstreamer-1.0 \
			                    -I$(TOPDIR)/src/util/gles

ifneq ($(findstring $(TARGET),$(debug)),)
CFLAGS                      += -DDEBUG=1
endif

LDFLAGS			            = -shared -lstdc++ -lm -ldl -Wl,--library-path=$(TARGET_PATH_LIB),-rpath-link=$(TARGET_PATH_LIB) \
			                    -lpthread -lgstbase-1.0 -lgstcontroller-1.0 -lgstreamer-1.0 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lglib-2.0 \
                                -lEGL -lGLESv2 -lX11

SRCS                        = $(wildcard ./*.c )
SRCS 		                += $(wildcard $(TOPDIR)/src/util/gles/*.c)

include $(TOPDIR)/makefile_c.env

