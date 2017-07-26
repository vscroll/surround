#include "stitchworker.h"
#include <opencv/cv.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/ipu.h>
#include "clpano2d.h"
#include "clpanorender.h"
#include "util.h"
#include "ICapture.h"
#include "imageshm.h"

using namespace cv;

StitchWorker::SourceSHMReadWorker::SourceSHMReadWorker(unsigned int channelIndex, ImageSHM* imageSHM)
{
	mChannelIndex = channelIndex;
    mImageSHM = imageSHM;
	mImage.data = NULL;
}


StitchWorker::SourceSHMReadWorker::~SourceSHMReadWorker()
{
}

void StitchWorker::SourceSHMReadWorker::run()
{
    if (NULL != mImageSHM)
    {
#if DEBUG_STITCH
        clock_t start = clock();
#endif
        if (mImageSHM->readSource(mImageBuf, sizeof(mImageBuf)) < 0)
        {
            return;
        }

        image_shm_header_t* header = (image_shm_header_t*)mImageBuf;
        mImage.info.width = header->width;
        mImage.info.height = header->height;
        mImage.info.pixfmt = header->pixfmt;
        mImage.info.size = header->size;
        mImage.timestamp = header->timestamp;
        mImage.data = mImageBuf + sizeof(image_shm_header_t);

#if DEBUG_STITCH
        std::cout << "SourceSHMReadWorker run: " 
                << " thread id:" << getTID()
                << ", runtime:" << (double)(clock()-start)/CLOCKS_PER_SEC
                << ", channel:" << mChannelIndex
				<< ", width:" << mImage.info.width
				<< ", height:" << mImage.info.height
				<< ", size:" << mImage.info.size
                << std::endl;
#endif
    }
}

StitchWorker::StitchWorker()
{
    mCapture = NULL;

    for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mImageSHM[i] = NULL;
        mSourceSHMReadWorker[i] = NULL;
    }

    pthread_mutex_init(&mInputImagesMutex, NULL);
    pthread_mutex_init(&mOutputPanoImageMutex, NULL);

    mPanoWidth = 0;
    mPanoHeight = 0;
    mPanoPixfmt = 0;
    mPanoSize = 0;

    mAccelPolicy = false;

    for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mLookupTab[i] = new cv::Mat();
    }

    mFBFd = -1;
    mFBMem = NULL;
    mFBSize = 0;
    memset(&mScreenInfo, 0, sizeof(mScreenInfo));

    mLastCallTime = 0;
}

StitchWorker::~StitchWorker()
{
    for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        if (NULL != mImageSHM[i])
        {
            mImageSHM[i]->destroy();
            delete mImageSHM[i];
            mImageSHM[i] = NULL;
        }

        if (NULL != mSourceSHMReadWorker[i])
        {
            mSourceSHMReadWorker[i]->stop();
            delete mSourceSHMReadWorker[i];
            mSourceSHMReadWorker[i] = NULL;
        }
    }

    for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        if (NULL != mLookupTab[i])
        {
            delete mLookupTab[i];
            mLookupTab[i] = NULL;
        }
    }
}

int StitchWorker::init(
            ICapture* capture,
            unsigned int inWidth,
		    unsigned int inHeight,
		    unsigned int inPixfmt,        
		    unsigned int panoWidth,
		    unsigned int panoHeight,
		    unsigned int panoPixfmt,
		    char* algoCfgFilePath,
		    int accelPolicy)
{
    mCapture = capture;
    if (NULL == capture)
    {
        for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
        {
            mImageSHM[i] = new ImageSHM();
            mImageSHM[i]->create((key_t)(SHM_FRONT_SOURCE_ID + i), SHM_FRONT_SOURCE_SIZE);
            mSourceSHMReadWorker[i] = new SourceSHMReadWorker(i, mImageSHM[i]);
            mSourceSHMReadWorker[i]->start(VIDEO_FPS_30);
        }
    }

    if (inPixfmt != V4L2_PIX_FMT_UYVY
        && inPixfmt != V4L2_PIX_FMT_YUYV)
    {
        return -1;
    }

    mPanoWidth = panoWidth;
    mPanoHeight = panoHeight;
    mPanoPixfmt = panoPixfmt;
    if (panoPixfmt == V4L2_PIX_FMT_RGB24
        || panoPixfmt == V4L2_PIX_FMT_BGR24)
    {
        mPanoSize = panoWidth*panoHeight*3;
    }
    else if (panoPixfmt == V4L2_PIX_FMT_UYVY
        || panoPixfmt == V4L2_PIX_FMT_YUYV)
    {
        mPanoSize = panoWidth*panoHeight*2;
    }
    else
    {
        return -1;
    }

    mAccelPolicy = accelPolicy;
    if (mAccelPolicy == ACCEL_POLICY_OPENCL)
    {
        mCLPano2D = new CLPano2D();
    }
    else if (mAccelPolicy == ACCEL_POLICY_OPENCL_RENDER)
    {
        mCLPano2D = new CLPanoRender();
    }
    else
    {
    }

    stitching_init(algoCfgFilePath,
        mLookupTab,
    	mMask,
    	mWeight,
        accelPolicy);

#if 0
    if (mEnableOpenCL)
    {
        // align to input image frame for CL: CL_MEM_USE_HOST_PTR
#if DEBUG_STITCH
        std::cout << "map rows:" << mStitchMap.rows
                << " cols:" << mStitchMap.cols
                << " channel:" << mStitchMap.channels()
                << std::endl;
        std::cout << "mask rows:" << mStitchMask.rows
                << " cols:" << mStitchMask.cols
                << " channel:" << mStitchMask.channels()
                << std::endl;
        std::cout << "height:" << inHeight
                << " width:" << inWidth
                << std::endl;
#endif
	    mStitchMapAlignX = Mat(inHeight, inWidth, CV_8UC3);
	    mStitchMapAlignY = Mat(inHeight, inWidth, CV_8UC3);
	    mStitchMaskAlign = Mat(inHeight, inWidth, CV_8UC3);

        for (int i = 0; i < mStitchMaskAlign.rows; i++)
        {
	        if (i >= mStitchMask.rows)
	        {
		        continue;
	        }

	        uchar* data = mStitchMaskAlign.ptr<uchar>(i);
            for (int j = 0; j < mStitchMaskAlign.cols; j++)
            {
	            if (j >= mStitchMask.cols)
	            {
		            continue;
	            }

		        *(data+j*3) = mStitchMask.ptr<uchar>(i)[j];
		        //*(data+j*3+1) = 0;
		        //*(data+j*3+2) = 0;
            }
        }

        for (int i = 0; i < mStitchMapAlignX.rows; i++)
        {
	        if (i >= mStitchMap.rows)
	        {
		        continue;
	        }

	        uchar* dataX = mStitchMapAlignX.ptr<uchar>(i);
	        uchar* dataY = mStitchMapAlignY.ptr<uchar>(i);
            for (int j = 0; j < mStitchMapAlignX.cols; j++)
            {
	            if (j >= mStitchMap.cols)
	            {
		            continue;
	            }

		        int x = mStitchMap.ptr<Point2f>(i)[j].x;
		        *(dataX+j*3) = x/255;
		        *(dataX+j*3+1) = x - (x/255)*255;
		        //*(dataX+j*3+2) = 0;

		        int y = mStitchMap.ptr<Point2f>(i)[j].y;
		        *(dataY+j*3) = y/255;
		        *(dataY+j*3+1) = y - (y/255)*255;
		        //*(dataY+j*3+2) = 0;
            }
        }
#if DEBUG_STITCH
        std::cout << "mStitchMapAlignX isContinuous:" << mStitchMapAlignX.isContinuous()
                << " rows:"<< mStitchMapAlignX.rows
		        << " cols:" << mStitchMapAlignX.cols
		        << " channel:" << mStitchMapAlignX.channels()
		        << std::endl;
        std::cout << "mStitchMapAlignY isContinuous:" << mStitchMapAlignY.isContinuous()
		        << " rows:"<< mStitchMapAlignY.rows
		        << " cols:" << mStitchMapAlignY.cols
		        << " channel:" << mStitchMapAlignY.channels()
                << std::endl;
        std::cout << "mStitchMaskAlign isContinuous:" << mStitchMaskAlign.isContinuous()
                << " rows:"<< mStitchMaskAlign.rows
                << " cols:" << mStitchMaskAlign.cols
                << " channel:" << mStitchMaskAlign.channels()
                << std::endl;
#endif
    }
#endif

    return 0;
}

void StitchWorker::run()
{
    clearOverstock();
#if DEBUG_STITCH
    int inputImageSize = 0;
    int panoSize = 0;

    clock_t start0 = clock();
    double elapsed_to_last = 0;
    if (mLastCallTime != 0)
    {
        elapsed_to_last = (double)(start0 - mLastCallTime)/CLOCKS_PER_SEC;
    }
    mLastCallTime = start0;
#endif

    unsigned char imageBuf[VIDEO_CHANNEL_SIZE][SHM_FRONT_SOURCE_SIZE] = {};

    surround_images_t* surroundImage = NULL;
    if (NULL != mCapture)
    {
        //one source may be come from ICapture Module
        surroundImage = mCapture->popOneFrame();
    }
    else
    {
        //one source may be come from share memory
        surroundImage = new surround_images_t();
        surroundImage->timestamp = 0;
#if 0
        for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
        {
            if (NULL != mImageSHM[i])
            {
                if (mImageSHM[i]->readSource(imageBuf[i], sizeof(imageBuf[i])) < 0)
                {
                    delete surroundImage;
                    surroundImage = NULL;
                    return;
                }

                image_shm_header_t* header = (image_shm_header_t*)imageBuf[i];
                surroundImage->frame[i].info.width = header->width;
                surroundImage->frame[i].info.height = header->height;
                surroundImage->frame[i].info.pixfmt = header->pixfmt;
                surroundImage->frame[i].info.size = header->size;
                surroundImage->frame[i].timestamp = header->timestamp;
                surroundImage->frame[i].data = imageBuf[i] + sizeof(image_shm_header_t);
                if (surroundImage->timestamp < surroundImage->frame[i].timestamp)
                {
                    surroundImage->timestamp = surroundImage->frame[i].timestamp;
                }
            }
        }
#endif
        bool isFull = true;
        for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
        {
            if (NULL != mSourceSHMReadWorker[i]
				&& NULL != mSourceSHMReadWorker[i]->mImage.data)
            {
                surroundImage->frame[i].info.width = mSourceSHMReadWorker[i]->mImage.info.width;
                surroundImage->frame[i].info.height = mSourceSHMReadWorker[i]->mImage.info.height;
                surroundImage->frame[i].info.pixfmt = mSourceSHMReadWorker[i]->mImage.info.pixfmt;
                surroundImage->frame[i].info.size = mSourceSHMReadWorker[i]->mImage.info.size;
                surroundImage->frame[i].timestamp = mSourceSHMReadWorker[i]->mImage.timestamp;
                surroundImage->frame[i].data = mSourceSHMReadWorker[i]->mImage.data;
                if (surroundImage->timestamp < surroundImage->frame[i].timestamp)
                {
                    surroundImage->timestamp = surroundImage->frame[i].timestamp;
                }
            }
            else
            {
                isFull = false;
            }
        }

        if (!isFull)
        {
            delete surroundImage;
            surroundImage = NULL;
        }
#if 0
        pthread_mutex_lock(&mInputImagesMutex);
        inputImageSize = mInputImagesQueue.size();
        if (inputImageSize > 0)
        {
            surroundImage = mInputImagesQueue.front();
            mInputImagesQueue.pop();
        }
        pthread_mutex_unlock(&mInputImagesMutex);
#endif
    }

    if (NULL == surroundImage)
    {
        return;
    }

#if DEBUG_STITCH
    clock_t start1 = clock();
#endif
    unsigned char* outPano = NULL;

    long timestamp = surroundImage->timestamp;
    long elapsed = Util::get_system_milliseconds() - surroundImage->timestamp;

    if (elapsed < 400)
    {
        surround_image_t* sideImage[VIDEO_CHANNEL_SIZE] = {NULL};
        for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
        {
            sideImage[i] = &(surroundImage->frame[i]);
        }

        outPano = new_pano_buffer(mAccelPolicy);
        if (mAccelPolicy == ACCEL_POLICY_OPENCL)
        {
            stitching_cl(sideImage,
                    mLookupTab,
                    mMask,
                    mWeight,
                    mPanoWidth,
                    mPanoHeight,
                    mPanoSize,
                    outPano);
        }
        else if (mAccelPolicy == ACCEL_POLICY_OPENCL_RENDER)
        {
            stitching_cl(sideImage,
                    mLookupTab,
                    mMask,
                    mWeight,
                    mScreenInfo.xres,
                    mScreenInfo.yres,
                    mFBSize,
                    outPano);
        }
        else if (mAccelPolicy == ACCEL_POLICY_CPU)
        {
            stitching(sideImage,
                    mLookupTab,
                    mMask,
                    mWeight,
                    mPanoWidth,
                    mPanoHeight,
                    mPanoSize,
                    outPano);
        }
        else
        {
            return;
        }
    }

#if DEBUG_STITCH
    clock_t start2 = clock();
#endif

    delete surroundImage;
    surroundImage = NULL;

    if (NULL != outPano
        && (mAccelPolicy == ACCEL_POLICY_OPENCL || mAccelPolicy == ACCEL_POLICY_CPU))
    {
        surround_image_t* tmp = new surround_image_t();
	    tmp->info.width = mPanoWidth;
	    tmp->info.height = mPanoHeight;
        tmp->info.pixfmt = mPanoPixfmt;
        tmp->info.size = mPanoSize;
        tmp->data = outPano;
        tmp->timestamp = timestamp;
        pthread_mutex_lock(&mOutputPanoImageMutex);
        mOutputPanoImageQueue.push(tmp);
#if DEBUG_STITCH
        panoSize = mOutputPanoImageQueue.size();
#endif
        pthread_mutex_unlock(&mOutputPanoImageMutex);
    }

#if DEBUG_STITCH

    std::cout << "StitchWorke::run"
            << " thread id:" << getTID()
            << ", elapsed to last time:" << elapsed_to_last
            << ", elapsed to capture:" << (double)elapsed/1000
            << ", read:" << (double)(start1-start0)/CLOCKS_PER_SEC
            << ", stitch:" << (double)(start2-start1)/CLOCKS_PER_SEC
            << ", input_size:" << inputImageSize
            << ", pano_size:" << panoSize
            << std::endl;
#endif
}

surround_image_t* StitchWorker::dequeuePanoImage()
{
    surround_image_t* image = NULL;
    pthread_mutex_lock(&mOutputPanoImageMutex);
    if (mOutputPanoImageQueue.size() > 0)
    {
        image = mOutputPanoImageQueue.front();
        mOutputPanoImageQueue.pop();
    }
	pthread_mutex_unlock(&mOutputPanoImageMutex);

    return image;
}

void StitchWorker::queueImages(surround_images_t* surroundImages)
{
    pthread_mutex_lock(&mInputImagesMutex);
    mInputImagesQueue.push(surroundImages);
    pthread_mutex_unlock(&mInputImagesMutex);
}

void StitchWorker::clearOverstock()
{
#if 0
    pthread_mutex_lock(&mInputImagesMutex);
    int size = mInputImagesQueue.size();
    if (size > OVERSTOCK_SIZE)
    {
        for (int i = 0; i < size; ++i)
        {
            struct surround_images_t* surroundImage = mInputImagesQueue.front();
            mInputImagesQueue.pop();
            if (NULL != surroundImage)
            {
                for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
                {
                    delete (unsigned char*)(surroundImage->frame[i].data);
                }
                delete surroundImage;
            }
        }
    }
    pthread_mutex_unlock(&mInputImagesMutex);
#endif

    pthread_mutex_lock(&mOutputPanoImageMutex);
    int outSize = mOutputPanoImageQueue.size();
    if (outSize > OVERSTOCK_SIZE)
    {
        for (int i = 0; i < outSize; ++i)
        {
            struct surround_image_t* surroundImage = mOutputPanoImageQueue.front();
            mOutputPanoImageQueue.pop();
            if (NULL != surroundImage)
            {
                delete (unsigned char*)(surroundImage->data);
                delete surroundImage;
            }
        }
    }
    pthread_mutex_unlock(&mOutputPanoImageMutex);
}

void StitchWorker::stitching_init(const std::string configPath,
        cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight,
        int accelPolicy)
{
#if DEBUG_STITCH
    std::cout << "System Initialization:" << configPath << std::endl;
#endif
	FileStorage fs(configPath, FileStorage::READ);
	fs["map1"] >> *(lookupTab[VIDEO_CHANNEL_FRONT]);
	fs["map2"] >> *(lookupTab[VIDEO_CHANNEL_REAR]);
	fs["map3"] >> *(lookupTab[VIDEO_CHANNEL_LEFT]);
	fs["map4"] >> *(lookupTab[VIDEO_CHANNEL_RIGHT]);

	fs["mask"] >> mask;
	fs["weight"] >> weight;
	fs.release();

    if (accelPolicy > ACCEL_POLICY_CPU)
    {
        char procPath[1024] = {0};
        if (Util::getAbsolutePath(procPath, 1024) >= 0)
        {
            char cfgPathName[1024] = {0};
            sprintf(cfgPathName, "%sstitch.cl", procPath);
            if (accelPolicy == ACCEL_POLICY_OPENCL)
            {
                mCLPano2D->init(cfgPathName, "stitch_2d");
            }
            else if (accelPolicy == ACCEL_POLICY_OPENCL_RENDER)
            {
                mCLPano2D->init(cfgPathName, "stitch_2d_render");
            }
            else
            {
            }
        }
    }

    return;
}

void StitchWorker::stitching(surround_image_t* sideImage[],
    	cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight,
        unsigned int panoWidth,
        unsigned int panoHeight,
        unsigned int panoSize,
        unsigned char* panoImage)
{
    unsigned char* front = (unsigned char*)(sideImage[VIDEO_CHANNEL_FRONT]->data);
    unsigned char* rear = (unsigned char*)(sideImage[VIDEO_CHANNEL_REAR]->data);
    unsigned char* left = (unsigned char*)(sideImage[VIDEO_CHANNEL_LEFT]->data);
    unsigned char* right = (unsigned char*)(sideImage[VIDEO_CHANNEL_RIGHT]->data);

    cv::Mat* lutFront = lookupTab[VIDEO_CHANNEL_FRONT];
    cv::Mat* lutRear = lookupTab[VIDEO_CHANNEL_REAR];
    cv::Mat* lutLeft = lookupTab[VIDEO_CHANNEL_LEFT];
    cv::Mat* lutRight = lookupTab[VIDEO_CHANNEL_RIGHT];
#if 0
	int width = sideImage[VIDEO_CHANNEL_REAR]->info.width;
	int height = sideImage[VIDEO_CHANNEL_REAR]->info.height;
	for (int j = 0; j < panoHeight; j++)
	{
		if (j >= height)
		{
			break;
		}

		for (int i = 0; i < panoWidth; i++)
		{
			if (i >= width)
			{
				break;
			}
			panoImage[j*panoWidth*2+i*2] = rear[j*width*2+i*2];
			panoImage[j*panoWidth*2+i*2+1] = rear[j*width*2+i*2+1];
		}
	}
#else
	for (int i = 0; i < panoSize; i++)
	{
		//424x600x2 value:0~8
		int flag = mask.ptr<uchar>(i)[0];
		switch (flag)
		{
			case 0:
			{
				float w = weight.ptr<float>(i)[0];
				size_t index1 = lutFront->ptr<float>(i)[0];
				size_t index2 = lutLeft->ptr<float>(i)[0];
				panoImage[i] = w*front[index1] + (1- w)*left[index2];
				break;
			}

			case 1:
			{
				size_t index1 = lutFront->ptr<float>(i)[0];
				panoImage[i] = front[index1];
				break;
			}unsigned char* new_pano_buffer(int accelPolicy);

			case 2:
			{
				float w = weight.ptr<float>(i)[0];
				size_t index1 = lutFront->ptr<float>(i)[0];
				size_t index2 = lutRight->ptr<float>(i)[0];
				panoImage[i] = w*front[index1] + (1 - w)*right[index2];
				break;
			}

			case 3:
			{
				size_t index1 = lutLeft->ptr<float>(i)[0];
				panoImage[i] = left[index1];
				break;
			}

			case 4:
			{
				panoImage[i] = 0;
				break;
			}

			case 5:
			{
				size_t index1 = lutRight->ptr<float>(i)[0];
				panoImage[i] = right[index1];
				break;
			}

			case 6:
			{
				float w = weight.ptr<float>(i)[0];
				size_t index1 = lutRear->ptr<float>(i)[0];
				size_t index2 = lutLeft->ptr<float>(i)[0];
				panoImage[i] = w*rear[index1] + (1 - w)*left[index2];
				break;
			}

			case 7:
			{
				size_t index1 = lutRear->ptr<float>(i)[0];
				panoImage[i] = rear[index1];
				break;
			}

			case 8:
			{
				float w = weight.ptr<float>(i)[0];
				size_t index1 = lutRear->ptr<float>(i)[0];
				size_t index2 = lutRight->ptr<float>(i)[0];
				panoImage[i] = w*rear[index1] + (1 - w)*right[index2];
				break;
			}
			default:
				break;
		}
	}
#endif
}

void StitchWorker::stitching_cl(surround_image_t* sideImage[],
    	cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight,
        unsigned int panoWidth,
        unsigned int panoHeight,
        unsigned int panoSize,
        unsigned char* panoImage)
{
    mCLPano2D->stitch(sideImage, lookupTab, mask, weight,
                panoWidth, panoHeight, panoSize, panoImage);
}

unsigned char* StitchWorker::new_pano_buffer(int accelPolicy)
{
    unsigned char* outPano = NULL;
    if (mAccelPolicy == ACCEL_POLICY_OPENCL
        || mAccelPolicy == ACCEL_POLICY_CPU)
    {
        outPano = new unsigned char[mPanoSize*sizeof(unsigned char)];
    }
    else if (mAccelPolicy == ACCEL_POLICY_OPENCL_RENDER)
    {
        if (NULL == mFBMem)
        {
            openFramebuffer(0);
        }
        
        outPano = (unsigned char*)mFBMem;
    }
    else
    {
    }

    return outPano;
}

int StitchWorker::openFramebuffer(int devIndex)
{
    char fb[16] = {0};
    sprintf(fb, "/dev/fb%d", devIndex);
    if ((mFBFd = open(fb, O_RDWR, 0)) < 0)
    {
	    return -1;
    }

    /* Get fix screen info. */
    struct fb_fix_screeninfo fbInfo;
    if (ioctl(mFBFd, FBIOGET_FSCREENINFO, &fbInfo) < 0)
    {
        std::cout << "FBIOGET_FSCREENINFO failed"
                  << std::endl;
	    return -1;
    }

    /* Get variable screen info. */
    if (ioctl(mFBFd, FBIOGET_VSCREENINFO, &mScreenInfo) < 0)
    {
        std::cout << "FBIOGET_VSCREENINFO failed"
                  << std::endl;
	    return -1;
    }

    mScreenInfo.bits_per_pixel = 16;
    if (mPanoPixfmt == V4L2_PIX_FMT_UYVY)
    {
        mScreenInfo.nonstd = IPU_PIX_FMT_UYVY;
    }
    else if (mPanoPixfmt == V4L2_PIX_FMT_YUYV)
    {
        mScreenInfo.nonstd = IPU_PIX_FMT_YUYV;
    }
    else
    {
    }

    if (ioctl(mFBFd, FBIOPUT_VSCREENINFO, &mScreenInfo) < 0)
    {
        std::cout << "FBIOPUT_VSCREENINFO failed"
                  << std::endl;
	    return -1;
    }

    mFBSize = mScreenInfo.xres_virtual * mScreenInfo.yres_virtual * mScreenInfo.bits_per_pixel / 8;

    /* Map the device to memory*/
    mFBMem = (unsigned short *)mmap(0, mFBSize, PROT_READ | PROT_WRITE, MAP_SHARED, mFBFd, 0);
    if ((int)mFBMem <= 0) {
        std::cout << "failed to map framebuffer device to memory"
                  << std::endl;
	    return -1;
    }

    std::cout << "RenderDevice::openDevice"
              << " fb:" << fb
              << " xres_virtual:" << mScreenInfo.xres_virtual
              << " yres_virtual:" << mScreenInfo.yres_virtual
              << ", xres:" << mScreenInfo.xres
              << ", yres:" << mScreenInfo.yres
              << std::endl;

    return 0;
}

void StitchWorker::closeFramebuffer()
{
    if (mFBFd > 0)
    {
        munmap(mFBMem, mFBSize);
        mFBMem = NULL;       
        close(mFBFd);
        mFBFd = -1;
    }
}
