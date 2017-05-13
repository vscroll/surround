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

int ImageSHM::writeImage(struct image_shm_header_t* header, unsigned char* data, unsigned int dataSize)
{
    void* shmAddr = mSHMUtil.getSHMAddr();
    if (NULL == shmAddr)
    {
	    return -1;
    }

    mSHMUtil.p_w();
    memcpy(shmAddr, header, sizeof(struct image_shm_header_t));
    memcpy(shmAddr+sizeof(struct image_shm_header_t), data, dataSize);
    mSHMUtil.v_r();

    return 0;
}

int ImageSHM::readImage(unsigned char* buf, unsigned int size)
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
