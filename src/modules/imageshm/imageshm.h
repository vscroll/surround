#ifndef IMAGESHM_H
#define IMAGESHM_H

#include "shmutil.h"

typedef struct image_shm_header_t {
    unsigned int width;
    unsigned int height;
    unsigned int pixfmt;
    unsigned int size;
    long timestamp;
} image_shm_header_t;

#define SHM_SIDE_ID 	0x8886
#define SHM_PANO2D_ID 	0x8887
#define SHM_SIDE_SIZE 	sizeof(image_shm_header_t) + 704*574*3
#define SHM_PANO2D_SIZE sizeof(image_shm_header_t) + 424*600*3

class ImageSHM
{
public:
    ImageSHM();
    virtual ~ImageSHM();
    int create(key_t key, unsigned int size);
    void destroy();
    int writeImage(struct image_shm_header_t* header, unsigned char* data, unsigned int dataSize);
    int readImage(unsigned char* buf, unsigned int size);

private:
    SHMUtil mSHMUtil;
};

#endif // IMAGESHM_H
