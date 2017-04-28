#before buildint it don't forget to export ROOTF_DIR and TOOLCHAIN (only if they are uncommented here)

# needs to source /opt/poky/x.x.x/environmentsetup.sh
TARGET			= libcontroller.so

RM			= rm -rf
 

#ROOTFS_DIR = $(SDKTARGETSYSROOT)
ROOTFS_DIR = $(HOME)/my-imx6/05_rootfs/distro-fsl-image

TARGET_PATH_LIB 	= $(ROOTFS_DIR)/usr/lib
TARGET_PATH_INCLUDE 	= $(ROOTFS_DIR)/usr/include 

COMMON_PATH_INCLUDE	= ../common
THREAD_PATH_INCLUDE	= ../thread
CAPTURE_PATH_INCLUDE	= ../capture
STITCH_PATH_INCLUDE	= ../stitch

CFLAGS			= -DLINUX -DUSE_SOC_MX6 -Wall -O2 -fsigned-char -march=armv7-a -mfpu=neon -mthumb-interwork -mtune=cortex-a9 -mfpu=vfp -mfloat-abi=hard \
			  -DEGL_API_FB -DGPU_TYPE_VIV -DGL_GLEXT_PROTOTYPES -DENABLE_GPU_RENDER_20 \
			  -fPIC \
			  -I. -I$(TARGET_PATH_INCLUDE) \
			  -I$(COMMON_PATH_INCLUDE) \
			  -I$(THREAD_PATH_INCLUDE) \
			  -I$(CAPTURE_PATH_INCLUDE) \
			  -I$(STITCH_PATH_INCLUDE) \
			  -I$(HOME)/my-imx6/02_source/linux-3.14.52/include/uapi \
			  -I$(HOME)/my-imx6/02_source/linux-3.14.52/tools/include

LFLAGS			= -Wl,--library-path=$(TARGET_PATH_LIB),-rpath-link=$(TARGET_PATH_LIB) -lm \
			   -lCLC -lOpenCL -lpthread -lopencv_core -lopencv_highgui

SHARE   	       	= -shared -o

SRCS 			= $(wildcard ./*.cpp )
SRCS 		       += $(wildcard ../common/*.cpp)
SRCS 		       += $(wildcard ../thread/*.cpp)
SRCS 		       += $(wildcard ../capture/*.cpp)
SRCS 		       += $(wildcard ../stitch/*.cpp)

OBJS 			= $(patsubst %.cpp, %.o, $(SRCS))

first: all

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) $(SHARE) $(LFLAGS) $^ -o $@  
  
$(OBJS):%.o : %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(TARGET) *.o

