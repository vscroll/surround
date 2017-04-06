#ifndef COMMON_H
#define COMMON_H

#include <QDebug>
#define DEBUG 1
#define DEBUG_CAPTURE 0
#define DEBUG_STITCH 1
#define DEBUG_UPDATE 1

#define CAPTURE_4_CHANNEL_ONCE 1
#define CAPTURE_ON_V4L2 1

#define DATA_TYPE_IPLIMAGE 0

#define DATA_FAKE 0

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

typedef struct surround_image1_t {
    void* image;
    double timestamp;
} surround_image1_t;

typedef struct surround_image4_t {
    void* image[VIDEO_CHANNEL_SIZE];
    double timestamp;
} surround_image4_t;

#endif // COMMON_H
