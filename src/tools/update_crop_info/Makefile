#before buildint it don't forget to export ROOTF_DIR and TOOLCHAIN (only if they are uncommented here)

# needs to source /opt/poky/x.x.x/environmentsetup.sh

TOPDIR                      = ../../..

TARGET                      = update_crop_info

LIBPATH                     = $(TOPDIR)/lib/
EXEPATH                     = $(TOPDIR)/bin/


#ROOTFS_DIR                 = $(SDKTARGETSYSROOT)
ROOTFS_DIR                  = $(HOME)/my-imx6/05_rootfs/distro-fsl-image
TARGET_PATH_LIB 	        = $(ROOTFS_DIR)/usr/lib
TARGET_PATH_INCLUDE         = $(ROOTFS_DIR)/usr/include 

CFLAGS			            = -DLINUX -DUSE_SOC_MX6 -Wall -O2 -fsigned-char -march=armv7-a -mfpu=neon -mthumb-interwork -mtune=cortex-a9 -mfpu=vfp -mfloat-abi=hard \
                                -DEGL_API_FB -DGPU_TYPE_VIV -DGL_GLEXT_PROTOTYPES -DENABLE_GPU_RENDER_20 \
			                    -I. -I$(TARGET_PATH_INCLUDE) \
                                -I$(HOME)/my-imx6/02_source/linux-3.14.52/include/uapi \
                                -I$(HOME)/my-imx6/02_source/linux-3.14.52/tools/include \
			                    -I$(TOPDIR)/src/include \
			                    -I$(TOPDIR)/src/util/ \
			                    -I$(TOPDIR)/src/util/ini \
			                    -I$(TOPDIR)/src/modules/config

LDFLAGS			            = -lstdc++ -lm -ldl -Wl,--library-path=$(TARGET_PATH_LIB),-rpath-link=$(TARGET_PATH_LIB) \
			                    -lpthread -lopencv_core -lopencv_highgui

SRCS 			            = $(wildcard ./*.cpp )
SRCS                       += $(wildcard $(TOPDIR)/src/modules/config/*.cpp)
SRCS                       += $(wildcard $(TOPDIR)/src/util/*.cpp)
SRCS                       += $(wildcard $(TOPDIR)/src/util/ini/*.cpp)

include $(TOPDIR)/makefile.env

