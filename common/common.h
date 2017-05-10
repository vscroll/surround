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

#define VIDEO_SIDE_RES_X_MAX 720
#define VIDEO_SIDE_RES_Y_MAX 576
#define VIDEO_PANO2D_RES_X_MAX 424
#define VIDEO_PANO2D_RES_Y_MAX 600

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

typedef struct cap_info_t {
  unsigned int in_pixfmt;
  unsigned int in_width;
  unsigned int in_height;
  unsigned int in_crop_x;
  unsigned int in_crop_y;
  unsigned int in_crop_width;
  unsigned int in_crop_height;
  unsigned int out_pixfmt;
  unsigned int out_width;
  unsigned int out_height;
} cap_info_t;

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
