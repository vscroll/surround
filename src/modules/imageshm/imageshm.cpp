#include "imageshm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

ImageSHM::ImageSHM()
{

}

ImageSHM::~ImageSHM()
{

}

int ImageSHM::create(key_t key, unsigned int size)
{
    return mSHMUtil.create(key, size);
}

void ImageSHM::destroy()
{
    mSHMUtil.destroy();
}


int ImageSHM::readSource(unsigned char* buf, unsigned int size)
{
    void* shmAddr = mSHMUtil.getSHMAddr();
    if (NULL == shmAddr)
    {
	    return -1;
    }

    mSHMUtil.p_r();
    memcpy(buf, shmAddr, size);
    mSHMUtil.v_w();

    return 0;
}

int ImageSHM::writeSource(surround_image_t* image)
{
    void* shmAddr = mSHMUtil.getSHMAddr();
    if (NULL == shmAddr)
    {
	    return -1;
    }

    mSHMUtil.p_w();

    struct image_shm_header_t header = {};
    header.channel = 0;
    header.width = image->info.width;
    header.height = image->info.height;
    header.pixfmt = image->info.pixfmt;
    header.size = image->info.size;
    header.timestamp = image->timestamp;
    unsigned char* frame = (unsigned char*)image->data;

    memcpy(shmAddr, &header, sizeof(struct image_shm_header_t));
    memcpy(shmAddr+sizeof(struct image_shm_header_t), frame, header.size);
    mSHMUtil.v_r();

    return 0;
}

int ImageSHM::writeFocusSource(surround_image_t* image, unsigned int focusChannelIndex)
{
    void* shmAddr = mSHMUtil.getSHMAddr();
    if (NULL == shmAddr)
    {
	    return -1;
    }

    mSHMUtil.p_w();

    struct image_shm_header_t header = {};
    header.channel = focusChannelIndex;
    header.width = image->info.width;
    header.height = image->info.height;
    header.pixfmt = image->info.pixfmt;
    header.size = image->info.size;
    header.timestamp = image->timestamp;
    unsigned char* frame = (unsigned char*)image->data;

    memcpy(shmAddr, &header, sizeof(struct image_shm_header_t));
    memcpy(shmAddr+sizeof(struct image_shm_header_t), frame, header.size);
    mSHMUtil.v_r();

    return 0;
}

int ImageSHM::writeAllSources(surround_images_t* image)
{
    void* shmAddr = mSHMUtil.getSHMAddr();
    if (NULL == shmAddr)
    {
	    return -1;
    }

    mSHMUtil.p_w();

    void* start = shmAddr;
    for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        struct image_shm_header_t header = {};
        header.channel = i;
        header.width = image->frame[i].info.width;
        header.height = image->frame[i].info.height;
        header.pixfmt = image->frame[i].info.pixfmt;
        header.size = image->frame[i].info.size;
        header.timestamp = image->frame[i].timestamp;
        unsigned char* frame = (unsigned char*)image->frame[i].data;

        memcpy(start, &header, sizeof(struct image_shm_header_t));
        start += sizeof(struct image_shm_header_t);
        memcpy(start, frame, header.size);
        start += header.size;
    }
    mSHMUtil.v_r();

    return 0;
}

int ImageSHM::writePanoSources(surround_image_t* image)
{
    void* shmAddr = mSHMUtil.getSHMAddr();
    if (NULL == shmAddr)
    {
	    return -1;
    }

    mSHMUtil.p_w();

    struct image_shm_header_t header = {};
    header.channel = 0;
    header.width = image->info.width;
    header.height = image->info.height;
    header.pixfmt = image->info.pixfmt;
    header.size = image->info.size;
    header.timestamp = image->timestamp;
    unsigned char* frame = (unsigned char*)image->data;

    memcpy(shmAddr, &header, sizeof(struct image_shm_header_t));
    memcpy(shmAddr+sizeof(struct image_shm_header_t), frame, header.size);
    mSHMUtil.v_r();

    return 0;
}

