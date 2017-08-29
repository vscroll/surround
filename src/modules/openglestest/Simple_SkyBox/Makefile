#before buildint it don't forget to export ROOTF_DIR and TOOLCHAIN (only if they are uncommented here)

# needs to source /opt/poky/x.x.x/environmentsetup.sh

TOPDIR                      = ../../../..

TARGET                      = Simple_SkyBox

LIBPATH                     = $(TOPDIR)/lib/
EXEPATH                     = $(TOPDIR)/bin/

#ROOTFS_DIR                 = $(SDKTARGETSYSROOT)
ROOTFS_DIR                  = $(HOME)/my-imx6/05_rootfs/distro-fsl-image
TARGET_PATH_LIB 	        = $(ROOTFS_DIR)/usr/lib
TARGET_PATH_INCLUDE         = $(ROOTFS_DIR)/usr/include
#TOOLCHAIN_DIR               = /opt/fsl-imx-x11/3.14.52-1.1.0/sysroots/cortexa9hf-vfp-neon-poky-linux-gnueabi
#TARGET_PATH_LIB 	        = $(TOOLCHAIN_DIR)/usr/lib
#TARGET_PATH_INCLUDE         = $(TOOLCHAIN_DIR)/usr/include 


CFLAGS			            = -DLINUX -DUSE_SOC_MX6 -Wall -O2 -fsigned-char -march=armv7-a -mfpu=neon -mthumb-interwork -mtune=cortex-a9 -mfpu=vfp -mfloat-abi=hard \
                                -DEGL_API_FB -DGPU_TYPE_VIV -DGL_GLEXT_PROTOTYPES -DENABLE_GPU_RENDER_20 \
			                    -I. -I$(TARGET_PATH_INCLUDE) \
			                    -I$(TOPDIR)/src/include \
			                    -I$(TOPDIR)/src/include/assimp \
			                    -I$(TOPDIR)/src/include/ImageMagick-6 \
			                    -I$(TOPDIR)/src/util/ogldev \
			                    -I$(TOPDIR)/src/util/gles

ifneq ($(BUILD_STATIC),)
LDFLAGS			            = -lstdc++ -lm -ldl -Wl,--library-path=$(TARGET_PATH_LIB),-rpath-link=$(TARGET_PATH_LIB) \
			                    -lpthread -lEGL -lGLESv2 -lX11 -L$(TOPDIR)/src/lib/ -lassimp -lIrrXML \
                                -lMagick++-6.Q16 -lMagickWand-6.Q16 -lMagickCore-6.Q16 -lXext -lpng16 -lxml2 -lz -lbz2 \
                                -ltiff -llzma -ljpeg -lpango-1.0 -lcairo -lpangocairo-1.0 -lgobject-2.0 -lfontconfig -lfreetype
else
LDFLAGS			            = -lstdc++ -lm -ldl -Wl,--library-path=$(TARGET_PATH_LIB),-rpath-link=$(TARGET_PATH_LIB) \
			                    -lpthread -lEGL -lGLESv2 -lX11 -L$(TOPDIR)/src/lib/shared -lassimp \
                                -lMagick++-6.Q16 -lMagickWand-6.Q16 -lMagickCore-6.Q16
endif

SRCS 			            = $(wildcard ./*.cpp)
SRCS 		                += $(wildcard $(TOPDIR)/src/util/ogldev/ogldev_util.cpp)
SRCS 		                += $(wildcard $(TOPDIR)/src/util/ogldev/ogldev_mesh.cpp)
SRCS 		                += $(wildcard $(TOPDIR)/src/util/ogldev/ogldev_texture.cpp)
SRCS 		                += $(wildcard $(TOPDIR)/src/util/ogldev/ogldev_cubemap_texture.cpp)
SRCS 		                += $(wildcard $(TOPDIR)/src/util/ogldev/ogldev_technique.cpp)
SRCS 		                += $(wildcard $(TOPDIR)/src/util/ogldev/ogldev_pipeline.cpp)
SRCS 		                += $(wildcard $(TOPDIR)/src/util/ogldev/ogldev_camera.cpp)
SRCS 		                += $(wildcard $(TOPDIR)/src/util/ogldev/ogldev_math_3d.cpp)
SRCS 		                += $(wildcard $(TOPDIR)/src/util/gles/*.c)

include $(TOPDIR)/makefile.env

