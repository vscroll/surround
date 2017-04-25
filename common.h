#ifndef COMMON_H
#define COMMON_H

#define DEBUG 1
#define DEBUG_CAPTURE 1
#define DEBUG_STITCH 1
#define DEBUG_UPDATE 1

#define CAPTURE_4_CHANNEL_ONCE 1
#define CAPTURE_ON_V4L2 1

#define DATA_FAKE 0

#define USE_IMX_IPU 1

#define IMX_OPENCL 1
#define IMX_OPENCL_ALLOC_ONCE 1

#define V4L2_BUF_COUNT 1

#define VIDEO_SIDE_RES_X_MAX 720
#define VIDEO_SIDE_RES_Y_MAX 576
#define VIDEO_PANO2D_RES_X_MAX 424
#define VIDEO_PANO2D_RES_Y_MAX 600

enum VIDEO_CHANNEL {
    VIDEO_CHANNEL_FRONT = 0,
    VIDEO_CHANNEL_REAR,
    VIDEO_CHANNEL_LEFT,
    VIDEO_CHANNEL_RIGHT,
    VIDEO_CHANNEL_SIZE
};

enum VIDEO_FPS {
    VIDEO_FPS_5 = 5,
    VIDEO_FPS_10 = 10,
    VIDEO_FPS_15 = 15,
    VIDEO_FPS_20 = 20,
    VIDEO_FPS_25 = 25,
    VIDEO_FPS_30 = 30
};

typedef struct surround_frame_t {
    void* data;
    unsigned int width;
    unsigned int height;
    unsigned int pixfmt;
} surround_frame_t;

typedef struct surround_image_t {
    surround_frame_t frame;
    double timestamp;
} surround_image_t;

typedef struct surround_images_t {
    surround_frame_t frame[VIDEO_CHANNEL_SIZE];
    double timestamp;
} surround_images_t;

#endif // COMMON_H
