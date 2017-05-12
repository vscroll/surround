#ifndef COMMON_H
#define COMMON_H

#include <linux/videodev2.h>

#define DEBUG 1
#define DEBUG_CAPTURE 1
#define DEBUG_STITCH 1
#define DEBUG_UPDATE 1

#define CAPTURE_4_CHANNEL_ONCE 1
#define CAPTURE_ON_V4L2 1

#define DATA_FAKE 0

#define USE_IMX_IPU 1

#define V4L2_BUF_COUNT 1

#define SIDE_RES_X_MAX 720
#define SIDE_RES_Y_MAX 576
#define PANO2D_RES_X_MAX 424
#define PANO2D_RES_Y_MAX 600

#define VIDEO_CHANNEL_FRONT	0
#define VIDEO_CHANNEL_REAR	1
#define VIDEO_CHANNEL_LEFT 	2
#define VIDEO_CHANNEL_RIGHT 	3
#define VIDEO_CHANNEL_SIZE 	4

#define VIDEO_FPS_5 		5
#define VIDEO_FPS_10		10
#define VIDEO_FPS_15		15
#define VIDEO_FPS_20		20
#define VIDEO_FPS_25		25
#define VIDEO_FPS_3		30

#define IN_PIX_FMT_UYVY 	V4L2_PIX_FMT_UYVY

#define OUT_PIX_FMT_BGR24 	V4L2_PIX_FMT_BGR24
#define OUT_PIX_FMT_UYVY 	V4L2_PIX_FMT_UYVY

typedef struct cap_sink_t {
  unsigned int pixfmt;
  unsigned int width;
  unsigned int height;
  unsigned int crop_x;
  unsigned int crop_y;
  unsigned int crop_w;
  unsigned int crop_h;
} cap_sink_t;

typedef struct cap_src_t {
  unsigned int pixfmt;
  unsigned int width;
  unsigned int height;
} cap_src_t;

typedef struct frame_info_t {
    unsigned int width;
    unsigned int height;
    unsigned int pixfmt;
} frame_info_t;

typedef struct surround_image_t {
    long timestamp;
    frame_info_t info;
    void* data;
} surround_image_t;

typedef struct surround_images_t {
    long timestamp;
    surround_image_t frame[VIDEO_CHANNEL_SIZE];
} surround_images_t;

#endif // COMMON_H
