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

#define SHM_SIDE_ID 	    0x8886
#define SHM_PANO_ID 	    0x8887
#define SHM_SIDE_DATA_SIZE  704*574*2
#define SHM_SIDE_SIZE 	    sizeof(image_shm_header_t) + SHM_SIDE_DATA_SIZE
#define SHM_PANO_DATA_SIZE  424*600*2
#define SHM_PANO_SIZE       sizeof(image_shm_header_t) + SHM_PANO_DATA_SIZE

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
