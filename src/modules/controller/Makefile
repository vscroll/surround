#before buildint it don't forget to export ROOTF_DIR and TOOLCHAIN (only if they are uncommented here)

# needs to source /opt/poky/x.x.x/environmentsetup.sh


TARGET                      = libcontroller.so
RM                          = rm -rf

PROJECT_ROOT                = ../../..
SRC_ROOT                    = $(PROJECT_ROOT)/src

#ROOTFS_DIR                 = $(SDKTARGETSYSROOT)
ROOTFS_DIR                  = $(HOME)/my-imx6/05_rootfs/distro-fsl-image
TARGET_PATH_LIB 	        = $(ROOTFS_DIR)/usr/lib
TARGET_PATH_INCLUDE         = $(ROOTFS_DIR)/usr/include 

CFLAGS			            = -DLINUX -DUSE_SOC_MX6 -Wall -O2 -fsigned-char -march=armv7-a -mfpu=neon -mthumb-interwork -mtune=cortex-a9 -mfpu=vfp -mfloat-abi=hard \
                                -DEGL_API_FB -DGPU_TYPE_VIV -DGL_GLEXT_PROTOTYPES -DENABLE_GPU_RENDER_20 \
			                    -fPIC \
			                    -I. -I$(TARGET_PATH_INCLUDE) \
                                -I$(HOME)/my-imx6/02_source/linux-3.14.52/include/uapi \
                                -I$(HOME)/my-imx6/02_source/linux-3.14.52/tools/include \
			                    -I$(SRC_ROOT)/include \
			                    -I$(SRC_ROOT)/util \
			                    -I$(SRC_ROOT)/util/thread \
			                    -I$(SRC_ROOT)/util/v4l2 \
			                    -I$(SRC_ROOT)/modules/capture


LFLAGS			            = -lstdc++ -lm -ldl -Wl,--library-path=$(TARGET_PATH_LIB),-rpath-link=$(TARGET_PATH_LIB) \
			                    -lpthread -lCLC -lOpenCL -lopencv_core -lopencv_highgui

SHARE   	       	        = -shared -o

SRCS 			            = $(wildcard ./*.cpp )
SRCS 		                += $(wildcard ../util/*.cpp)
SRCS 		                += $(wildcard ../thread/*.cpp)
SRCS 		                += $(wildcard ../v4l2/*.cpp)
SRCS 		                += $(wildcard ../modules/capture/*.cpp)

OBJS 			            = $(patsubst %.cpp, %.o, $(SRCS))

first: all

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) $(SHARE) $(LFLAGS) $^ -o $@  
  
$(OBJS):%.o : %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(TARGET) *.o

