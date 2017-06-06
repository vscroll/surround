#include "stitchworker.h"
#include <opencv/cv.h>
#include <string.h>
#include "clpano2d.h"
#include "util.h"
#include "ICapture.h"
#include "imageshm.h"

using namespace cv;

StitchWorker::StitchWorker()
{
    mCapture = NULL;

    for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mImageSHM[i] = NULL;
    }

    pthread_mutex_init(&mInputImagesMutex, NULL);
    pthread_mutex_init(&mOutputPanoImageMutex, NULL);

    mPanoWidth = 0;
    mPanoHeight = 0;
    mPanoPixfmt = 0;
    mPanoSize = 0;

    mEnableOpenCL = false;
    mCLPano2D = new CLPano2D();

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
        mLutFront,
    	mLutRear,
    	mLutLeft,
    	mLutRight,
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
#if DEBUG_STITCH
    int inputImageSize = 0;
    int panoSize = 0;

    clock_t start = clock();
    double elapsed_to_last = 0;
    if (mLastCallTime != 0)
    {
        elapsed_to_last = (double)(start - mLastCallTime)/CLOCKS_PER_SEC;
    }
    mLastCallTime = start;
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

    unsigned char* outPano = new unsigned char[mPanoSize*sizeof(unsigned char)];

    long timestamp = surroundImage->timestamp;
    long elapsed = Util::get_system_milliseconds() - surroundImage->timestamp;

    if (elapsed < 400)
    {
        if (mEnableOpenCL)
        {
            /*stitching_cl(surroundImage->frame[VIDEO_CHANNEL_FRONT].data,
                         surroundImage->frame[VIDEO_CHANNEL_REAR].data,
                         surroundImage->frame[VIDEO_CHANNEL_LEFT].data,
                         surroundImage->frame[VIDEO_CHANNEL_RIGHT].data,
                         mStitchMapAlignX,
                         mStitchMapAlignY,
                         mStitchMaskAlign,
                         &outPano,
                         mPanoWidth,
                         mPanoHeight);*/
        }
        else
        {
            stitching((unsigned char*)surroundImage->frame[VIDEO_CHANNEL_FRONT].data,
                    (unsigned char*)surroundImage->frame[VIDEO_CHANNEL_REAR].data,
                    (unsigned char*)surroundImage->frame[VIDEO_CHANNEL_LEFT].data,
                    (unsigned char*)surroundImage->frame[VIDEO_CHANNEL_RIGHT].data,
                    mLutFront,
                    mLutRear,
                    mLutLeft,
                    mLutRight,
                    mMask,
                    mWeight,
                    outPano,
                    mPanoWidth,
                    mPanoHeight,
                    mPanoSize);
        }
    }
#if DEBUG_STITCH
       clock_t end = clock();
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
            << ", stitch:" << (double)(end-start)/CLOCKS_PER_SEC
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
    pthread_mutex_lock(&mInputImagesMutex);
    int size = mInputImagesQueue.size();
    if (size > 5)
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

    pthread_mutex_lock(&mOutputPanoImageMutex);
    size = mOutputPanoImageQueue.size();
    if (size > 5)
    {
        for (int i = 0; i < size; ++i)
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

void StitchWorker::stitching_init(
        const std::string configPath,
        cv::Mat& LutFront,
    	cv::Mat& LutRear,
    	cv::Mat& LutLeft,
    	cv::Mat& LutRight,
    	cv::Mat& mask,
    	cv::Mat& weight,
        bool enableOpenCL)
{
#if DEBUG_STITCH
    std::cout << "System Initialization:" << configPath << std::endl;
#endif
	FileStorage fs(configPath, FileStorage::READ);
	fs["map1"] >> LutFront;
	fs["map2"] >> LutRear;
	fs["map3"] >> LutLeft;
	fs["map4"] >> LutRight;

	fs["mask"] >> mask;
	fs["weight"] >> weight;
	fs.release();

    if (enableOpenCL)
    {
        mCLPano2D->init("stitch.cl", "stitch_2d");
    }

    return;
}

void StitchWorker::stitching(
        const unsigned char* front,
        const unsigned char* rear,
    	const unsigned char* left,
    	const unsigned char* right,
    	const cv::Mat& LutFront,
    	const cv::Mat& LutRear,
    	const cv::Mat& LutLeft,
    	const cv::Mat& LutRight,
    	const cv::Mat& mask,
    	const cv::Mat& weight,
        unsigned char* outPano,
        unsigned int outPanoWidth,
        unsigned int outPanoHeight,
        unsigned int outPanoSize)
{
    if (NULL == front || NULL == rear || NULL == left || NULL == right)
    {
        return;
    }

	for (int i = 0; i < outPanoSize; i++)
	{
		//424x600x2 value:0~8
		int flag = mask.ptr<uchar>(i)[0];
		switch (flag)
		{
			case 0:
			{
				float w = 0.5;//float w = weight.ptr<uchar>(i)[0];
				size_t index1 = LutFront.ptr<float>(i)[0];
				size_t index2 = LutLeft.ptr<float>(i)[0];
				outPano[i] = /*front[index1];*/w*front[index1] + (1- w)*left[index2];
				break;
			}

			case 1:
			{
				size_t index1 = LutFront.ptr<float>(i)[0];
				outPano[i] = front[index1];
				break;
			}

			case 2:
			{
				float w = 0.5;//float w = weight.ptr<uchar>(i)[0];
				size_t index1 = LutFront.ptr<float>(i)[0];
				size_t index2 = LutRight.ptr<float>(i)[0];
				outPano[i] = /*front[index1];*/w*front[index1] + (1 - w)*right[index2];
				break;
			}

			case 3:
			{
				size_t index1 = LutLeft.ptr<float>(i)[0];
				outPano[i] = left[index1];
				break;
			}

			case 4:
			{
				outPano[i] = 0;
				break;
			}

			case 5:
			{
				size_t index1 = LutRight.ptr<float>(i)[0];
				outPano[i] = right[index1];
				break;
			}

			case 6:
			{
				float w = 0.5;//float w = weight.ptr<uchar>(i)[0];
				size_t index1 = LutRear.ptr<float>(i)[0];
				size_t index2 = LutLeft.ptr<float>(i)[0];
				outPano[i] = /*rear[index1];*/w*rear[index1] + (1 - w)*left[index2];
				break;
			}

			case 7:
			{
				size_t index1 = LutRear.ptr<float>(i)[0];
				outPano[i] = rear[index1];
				break;
			}

			case 8:
			{
				float w = 0.5;//float w = weight.ptr<uchar>(i)[0];
				size_t index1 = LutRear.ptr<float>(i)[0];
				size_t index2 = LutRight.ptr<float>(i)[0];
				outPano[i] =/* rear[index1];*/w*rear[index1] + (1 - w)*right[index2];
				break;
			}
			default:
				break;
		}
	}
}

void StitchWorker::stitching_cl(const void* front, const void* rear, const void* left, const void* right,
                const cv::Mat& mapX, const cv::Mat& mapY, const cv::Mat& mask,
                void** outPano2D, int outPano2DWidth, int outPano2DHeight)
{
    if (NULL == front || NULL == rear || NULL == left || NULL == right)
    {
        return;
    }

    std::vector<cv::Mat> fishImgs;
    cv::Mat matFront(*(cv::Mat*)front);
    cv::Mat matRear(*(cv::Mat*)rear);
    cv::Mat matLeft(*(cv::Mat*)left);
    cv::Mat matRight(*(cv::Mat*)right);

    fishImgs.push_back(matFront);
    fishImgs.push_back(matRear);
    fishImgs.push_back(matLeft);
    fishImgs.push_back(matRight);

    *outPano2D = new Mat(outPano2DHeight, outPano2DWidth, CV_8UC3);
    mCLPano2D->stitch_cl_2d(fishImgs, mapX, mapY, mask, *((cv::Mat*)*outPano2D));
}
