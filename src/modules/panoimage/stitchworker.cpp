#include "stitchworker.h"
#include <opencv/cv.h>
#include <string.h>
#include "clpano2d.h"
#include "util.h"
#include "ICapture.h"
#include "imageshm.h"

using namespace cv;

StitchWorker::SourceSHMReadWorker::SourceSHMReadWorker(ImageSHM* imageSHM)
{
    mImageSHM = imageSHM;
}


StitchWorker::SourceSHMReadWorker::~SourceSHMReadWorker()
{
}

void StitchWorker::SourceSHMReadWorker::run()
{
    if (NULL != mImageSHM)
    {
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

    mEnableOpenCL = false;
    mCLPano2D = new CLPano2D();

    for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mLookupTab[i] = new cv::Mat();
    }

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
		    bool enableOpenCL)
{
    mCapture = capture;
    if (NULL == capture)
    {
        for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
        {
            mImageSHM[i] = new ImageSHM();
            mImageSHM[i]->create((key_t)(SHM_FRONT_SOURCE_ID + i), SHM_FRONT_SOURCE_SIZE);
            mSourceSHMReadWorker[i] = new SourceSHMReadWorker(mImageSHM[i]);
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

    mEnableOpenCL = enableOpenCL;

    stitching_init(algoCfgFilePath,
        mLookupTab,
    	mMask,
    	mWeight,
        mEnableOpenCL);

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
                && NULL != mSourceSHMReadWorker[i])
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
    unsigned char* outPano = new unsigned char[mPanoSize*sizeof(unsigned char)];

    long timestamp = surroundImage->timestamp;
    long elapsed = Util::get_system_milliseconds() - surroundImage->timestamp;

    if (elapsed < 400)
    {
        surround_image_t* sideImage[VIDEO_CHANNEL_SIZE] = {NULL};
        for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
        {
            sideImage[i] = &(surroundImage->frame[i]);
        }

        if (mEnableOpenCL)
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
        else
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
    }

#if DEBUG_STITCH
    clock_t start2 = clock();
#endif

    delete surroundImage;
    surroundImage = NULL;

    if (NULL != outPano)
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
        bool enableOpenCL)
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

    if (enableOpenCL)
    {
        char procPath[1024] = {0};
        if (Util::getAbsolutePath(procPath, 1024) >= 0)
        {
            char cfgPathName[1024] = {0};
            sprintf(cfgPathName, "%sstitch.cl", procPath);
            mCLPano2D->init(cfgPathName, "stitch_2d");
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
			}

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
