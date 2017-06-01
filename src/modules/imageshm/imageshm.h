#ifndef IMAGESHM_H
#define IMAGESHM_H

#include "common.h"
#include "shmutil.h"

typedef struct image_shm_header_t {
    unsigned int channel;    
    unsigned int width;
    unsigned int height;
    unsigned int pixfmt;
    unsigned int size;
    long timestamp;
} image_shm_header_t;

#define SHM_FOCUS_SOURCE_ID 	    0x8880
#define SHM_FRONT_SOURCE_ID 	    0x8881
#define SHM_REAR_SOURCE_ID 	        (SHM_FRONT_SOURCE_ID + 1)
#define SHM_LEFT_SOURCE_ID 	        (SHM_FRONT_SOURCE_ID + 2)
#define SHM_RIGHT_SOURCE_ID 	    (SHM_FRONT_SOURCE_ID + 3)
#define SHM_ALL_SOURCES_ID 	        0x8885
#define SHM_PANO_SOURCE_ID 	        0x8887

#define SHM_FOCUS_SOURCE_DATA_SIZE      (CAPTURE_VIDEO_RES_X*CAPTURE_VIDEO_RES_Y*2)
#define SHM_FOCUS_SOURCE_SIZE           (sizeof(image_shm_header_t) + SHM_FOCUS_SOURCE_DATA_SIZE)

#define SHM_FRONT_SOURCE_DATA_SIZE      (CAPTURE_VIDEO_RES_X*CAPTURE_VIDEO_RES_Y*2)
#define SHM_FRONT_SOURCE_SIZE           (sizeof(image_shm_header_t) + SHM_FRONT_SOURCE_DATA_SIZE)

#define SHM_REAR_SOURCE_DATA_SIZE       SHM_FRONT_SOURCE_DATA_SIZE
#define SHM_REAR_SOURCE_SIZE            SHM_FRONT_SOURCE_SIZE

#define SHM_LEFT_SOURCE_DATA_SIZE       SHM_FRONT_SOURCE_DATA_SIZE
#define SHM_LEFT_SOURCE_SIZE            SHM_FRONT_SOURCE_SIZE

#define SHM_RIGHT_SOURCE_DATA_SIZE      SHM_FRONT_SOURCE_DATA_SIZE
#define SHM_RIGHT_SOURCE_SIZE           SHM_FRONT_SOURCE_SIZE

#define SHM_ALL_SOURCES_DATA_SIZE       (CAPTURE_VIDEO_RES_X*CAPTURE_VIDEO_RES_Y*2)
#define SHM_ALL_SOURCES_SIZE            4*(sizeof(image_shm_header_t) + SHM_ALL_SOURCES_DATA_SIZE)

#define SHM_PANO_SOURCE_DATA_SIZE       (RENDER_VIDEO_RES_PANO_X*RENDER_VIDEO_RES_PANO_Y*2)
#define SHM_PANO_SOURCE_SIZE            (sizeof(image_shm_header_t) + SHM_PANO_SOURCE_DATA_SIZE)

class ImageSHM
{
public:
    ImageSHM();
    virtual ~ImageSHM();
    int create(key_t key, unsigned int size);
    void destroy();
    int readSource(unsigned char* buf, unsigned int size);
    int writeSource(surround_image_t* image);
    int writeFocusSource(surround_image_t* image, unsigned int focusChannelIndex);
    int writeAllSources(surround_images_t* image);
    int writePanoSources(surround_image_t* image);

private:
    SHMUtil mSHMUtil;
};

#endif // IMAGESHM_H
