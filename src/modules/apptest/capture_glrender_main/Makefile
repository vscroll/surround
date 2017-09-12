#before buildint it don't forget to export ROOTF_DIR and TOOLCHAIN (only if they are uncommented here)

# needs to source /opt/poky/x.x.x/environmentsetup.sh

TOPDIR                      = ../../../..

TARGET                      = capture_glrender_main

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
			                    -I$(TOPDIR)/src/util \
			                    -I$(TOPDIR)/src/util/thread \
			                    -I$(TOPDIR)/src/util/v4l2 \
			                    -I$(TOPDIR)/src/modules/capture \
			                    -I$(TOPDIR)/src/util/shm \
			                    -I$(TOPDIR)/src/modules/imageshm \
			                    -I$(TOPDIR)/src/modules/controller \
			                    -I$(TOPDIR)/src/modules/glrender \
			                    -I$(TOPDIR)/src/util/ini \
			                    -I$(TOPDIR)/src/modules/config \
			                    -I$(TOPDIR)/src/util/gles \
			                    -I$(TOPDIR)/src/util/ogldev

ifneq ($(findstring $(TARGET),$(debug)),)
CFLAGS                      += -DDEBUG=1
endif

LDFLAGS			            = -lstdc++ -lm -ldl -Wl,--library-path=$(TARGET_PATH_LIB),-rpath-link=$(TARGET_PATH_LIB) \
			                    -lpthread -lEGL -lGLESv2 -lX11 -lopencv_core

SRCS 			            = $(wildcard ./*.cpp )
SRCS 		                += $(wildcard $(TOPDIR)/src/util/*.cpp)
SRCS 		                += $(wildcard $(TOPDIR)/src/util/thread/*.cpp)
SRCS 		                += $(wildcard $(TOPDIR)/src/util/v4l2/*.cpp)
SRCS 		                += $(wildcard $(TOPDIR)/src/modules/capture/*.cpp)
SRCS 		                += $(wildcard $(TOPDIR)/src/util/shm/*.cpp)
SRCS 		                += $(wildcard $(TOPDIR)/src/modules/imageshm/*.cpp)
SRCS 		                += $(wildcard $(TOPDIR)/src/modules/glrender/*.cpp)
SRCS 		                += $(wildcard $(TOPDIR)/src/modules/config/*.cpp)
SRCS                        += $(TOPDIR)/src/modules/controller/controller.cpp
SRCS                        += $(TOPDIR)/src/modules/controller/controllergl.cpp
SRCS 		                += $(wildcard $(TOPDIR)/src/util/ini/*.cpp)
SRCS 		                += $(wildcard $(TOPDIR)/src/util/gles/*.c)
SRCS 		                += $(wildcard $(TOPDIR)/src/util/ogldev/ogldev_util.cpp)

include $(TOPDIR)/makefile.env

