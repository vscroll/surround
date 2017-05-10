#include "stitchworker.h"
#include "ICapture.h"
#include <opencv/cv.h>
#include <string>
#include "clpano2d.h"

using namespace cv;

StitchWorker::StitchWorker()
{
    mCurChannelIndex = VIDEO_CHANNEL_FRONT;
    mLastTimestamp = 0.0;
    mEnableOpenCL = false;
    mCLPano2D = new CLPano2D();

    pthread_mutex_init(&mOutputPano2DImageMutex, NULL);
    pthread_mutex_init(&mOutputSideImageMutex, NULL);
}

StitchWorker::~StitchWorker()
{

}

void StitchWorker::init(ICapture *capture,	
		unsigned int pano2DWidth,
		unsigned int pano2DHeight,
		char* configFilePath,
		bool enableOpenCL)
{
    if (NULL == capture)
    {
        return;
    }

    mPano2DWidth = pano2DWidth;
    mPano2DHeight = pano2DHeight;
    mEnableOpenCL = enableOpenCL;

    stitching_init(configFilePath, mStitchMap, mStitchMask, mEnableOpenCL);

    if (mEnableOpenCL)
    {
        // align to input image frame for CL: CL_MEM_USE_HOST_PTR
	unsigned int width = 0;
	unsigned int height = 0;
	capture->getResolution(VIDEO_CHANNEL_FRONT, &width, &height);
#if DEBUG_STITCH
        std::cout << "map rows:" << mStitchMap.rows  << " cols:" << mStitchMap.cols  << " channel:" << mStitchMap.channels() << std::endl;
        std::cout << "mask rows:" << mStitchMask.rows  << " cols:" << mStitchMask.cols  << " channel:" << mStitchMask.channels() << std::endl;
        std::cout << "height:" << height  << " width:" << width  << std::endl;
#endif
	mStitchMapAlignX = Mat(height, width, CV_8UC3);
	mStitchMapAlignY = Mat(height, width, CV_8UC3);
	mStitchMaskAlign = Mat(height, width, CV_8UC3);

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

#if 0
    uchar* data0 = mStitchMaskAlign.ptr<uchar>(0);
    uchar* dataX0 = mStitchMapAlignX.ptr<uchar>(0);
    uchar* dataY0 = mStitchMapAlignY.ptr<uchar>(0);
    for (int i = 0; i < 3; i++)
    {
        uchar* data = mStitchMaskAlign.ptr<uchar>(i);
        uchar* dataX = mStitchMapAlignX.ptr<uchar>(i);
        uchar* dataY = mStitchMapAlignY.ptr<uchar>(i);
        for (int j = 0; j < mStitchMaskAlign.cols; j++)
        {
	    printf("\n %d %d", i, j);
            printf(" org: %d mask0:%d %d %d mask1:%d %d %d",
		    mStitchMask.ptr<uchar>(i)[j],
		    *(data0+i*mStitchMaskAlign.cols*3+j*3),
		    *(data0+i*mStitchMaskAlign.cols*3+j*3+1),
                    *(data0+i*mStitchMaskAlign.cols*3+j*3+2),
		    *(data+j*3), *(data+j*3+1), *(data+j*3+2));

            printf(" org: %d mapx0:%d %d %d mapx1:%d %d %d",
                    (int)(mStitchMap.ptr<Point2f>(i)[j].x),
		    *(dataX0+i*mStitchMapAlignX.cols*3+j*3),
		    *(dataX0+i*mStitchMapAlignX.cols*3+j*3+1),
                    *(dataX0+i*mStitchMapAlignX.cols*3+j*3+2),
                    *(dataX+j*3), *(dataX+j*3+1), *(dataX+j*3+2));

            printf(" org: %d mapy0:%d %d %d mapy1:%d %d %d\n",
                    (int)(mStitchMap.ptr<Point2f>(i)[j].y),
		    *(dataY0+i*mStitchMapAlignY.cols*3+j*3),
		    *(dataY0+i*mStitchMapAlignY.cols*3+j*3+1),
                    *(dataY0+i*mStitchMapAlignY.cols*3+j*3+2),
                    *(dataY+j*3), *(dataY+j*3+1), *(dataY+j*3+2));
        }
    }
#endif

    mCapture = capture;
}

void StitchWorker::run()
{
    double timestamp = 0;

    surround_images_t* surroundImage = mCapture->popOneFrame();
    if (NULL == surroundImage)
    {
        return;
    }

    void* outSide = NULL;
    void* outPano2D = NULL;
#if DEBUG_STITCH
    int pano2d_size = 0;
    int side_size = 0;
    double end = 0.0;
#endif
    timestamp = surroundImage->timestamp;
    double elapsed = 0;
    double start = clock();
    elapsed = ((start - surroundImage->timestamp)/CLOCKS_PER_SEC);

#if DEBUG_STITCH
    double elapsed_to_last = 0;
    if (mLastTimestamp > 0.00001f)
    {
        elapsed_to_last = (start - mLastTimestamp)/CLOCKS_PER_SEC;
    }
    mLastTimestamp = start;
#endif

    if ((int)(elapsed*1000) < 500)
    {
        if (mEnableOpenCL)
        {
            stitching_cl(surroundImage->frame[VIDEO_CHANNEL_FRONT].data,
                         surroundImage->frame[VIDEO_CHANNEL_REAR].data,
                         surroundImage->frame[VIDEO_CHANNEL_LEFT].data,
                         surroundImage->frame[VIDEO_CHANNEL_RIGHT].data,
                         mStitchMapAlignX,
                         mStitchMapAlignY,
                         mStitchMaskAlign,
                         &outPano2D,
                         mPano2DWidth,
                         mPano2DHeight,
                         &outSide,
                         surroundImage->frame[mCurChannelIndex].width,
                         surroundImage->frame[mCurChannelIndex].height,
                         mCurChannelIndex);
        }
        else
        {
            stitching(surroundImage->frame[VIDEO_CHANNEL_FRONT].data,
                      surroundImage->frame[VIDEO_CHANNEL_REAR].data,
                      surroundImage->frame[VIDEO_CHANNEL_LEFT].data,
                      surroundImage->frame[VIDEO_CHANNEL_RIGHT].data,
                      mStitchMap,
                      mStitchMask,
                      &outPano2D,
                      mPano2DWidth,
                      mPano2DHeight,
                      &outSide,
                      surroundImage->frame[mCurChannelIndex].width,
                      surroundImage->frame[mCurChannelIndex].height,
                      mCurChannelIndex);
        }
    }
#if DEBUG_STITCH
    end = clock();
#endif

    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        if (NULL != surroundImage->frame[i].data)
        {
            delete (cv::Mat*)surroundImage->frame[i].data;
        }
    }

    delete surroundImage;
    surroundImage = NULL;

    if (NULL != outPano2D)
    {
        surround_image_t* tmp = new surround_image_t();
        tmp->frame.data = outPano2D;
        tmp->timestamp = timestamp;
        pthread_mutex_lock(&mOutputPano2DImageMutex);
        mOutputPano2DImageQueue.push(tmp);
#if DEBUG_STITCH
        pano2d_size = mOutputPano2DImageQueue.size();
#endif
        pthread_mutex_unlock(&mOutputPano2DImageMutex);
    }

    if (NULL != outSide)
    {
        surround_image_t* tmp = new surround_image_t();
        tmp->frame.data = outSide;
        tmp->timestamp = timestamp;
        pthread_mutex_lock(&mOutputSideImageMutex);
        mOutputSideImageQueue.push(tmp);
#if DEBUG_STITCH
        side_size = mOutputSideImageQueue.size();
#endif
        pthread_mutex_unlock(&mOutputSideImageMutex);
    }

#if DEBUG_STITCH

    std::cout << "StitchWorke::run"
	     << " thread id:" << getTID()
             <<", elapsed to last time:" << elapsed_to_last
            << ", elapsed to capture:" << elapsed
            << ", stitch:" << (end-start)/CLOCKS_PER_SEC
            << ", pano2d_size:" << pano2d_size
            << ", channel:" << mCurChannelIndex
            << ", side_size:" << side_size
	    << std::endl;
#endif

}

surround_image_t* StitchWorker::dequeuePano2DImage()
{
    surround_image_t* image = NULL;
    {
        pthread_mutex_lock(&mOutputPano2DImageMutex);;
        if (mOutputPano2DImageQueue.size() > 0)
        {
            image = mOutputPano2DImageQueue.front();
	    mOutputPano2DImageQueue.pop();
        }
	pthread_mutex_unlock(&mOutputPano2DImageMutex);
    }
    return image;
}

surround_image_t* StitchWorker::dequeueSideImage(unsigned int  channel)
{
    if (mCurChannelIndex != channel
            && channel < VIDEO_CHANNEL_SIZE)
    {
        mCurChannelIndex = channel;
    }

    surround_image_t* image = NULL;
    {
        pthread_mutex_lock(&mOutputSideImageMutex);
        if (mOutputSideImageQueue.size() > 0)
        {
            image = mOutputSideImageQueue.front();
	    mOutputSideImageQueue.pop();
        }
	pthread_mutex_unlock(&mOutputSideImageMutex);
    }
    return image;
}

void StitchWorker::stitching_init(const string config_path, Mat& map, Mat& mask, bool enableOpenCL)
{
#if DEBUG_STITCH
    std::cout << "System Initialization:" << config_path << std::endl;
#endif
    FileStorage fs(config_path, FileStorage::READ);
    fs["Map"] >> map;
    fs["Mask"] >> mask;
    fs.release();

    if (enableOpenCL)
    {
        mCLPano2D->init("stitch.cl", "stitch_2d");
    }

    return;
}

void StitchWorker::stitching(const void* front, const void* rear, const void* left, const void* right,
               const cv::Mat& map, const cv::Mat& mask,
               void** outPano2D, int outPano2DWidth, int outPano2DHeight,
               void** outSide, int outSideWidth, int outSideHeight, int outSideChannel)
{
    if (NULL == front || NULL == rear || NULL == left || NULL == right)
    {
        return;
    }

#if DATA_FAKE

    switch (channel)
    {
        case VIDEO_CHANNEL_FRONT:
            {
                *outPano2D = new cv::Mat(*((cv::Mat*)front));
                *outSide = new cv::Mat(*((cv::Mat*)front));
            }
            break;
        case VIDEO_CHANNEL_REAR:
            {
                *outPano2D = new cv::Mat(*((cv::Mat*)rear));
                *outSide = new cv::Mat(*((cv::Mat*)rear));
            }
            break;
        case VIDEO_CHANNEL_LEFT:
            {
                *outPano2D = new cv::Mat(*((cv::Mat*)left));
                *outSide = new cv::Mat(*((cv::Mat*)left));
            }
            break;
        case VIDEO_CHANNEL_RIGHT:
            {
                *outPano2D = new cv::Mat(*((cv::Mat*)right));
                *outSide = new cv::Mat(*((cv::Mat*)right));
            }
            break;
        default:
            break;
    }

#else

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
    for (int i = 0; i < outPano2DHeight; i++)
    {
        for (int j = 0; j < outPano2DWidth; j++)
        {
            int flag = mask.ptr<uchar>(i)[j];
            int x = map.ptr<Point2f>(i)[j].x;
            int y = map.ptr<Point2f>(i)[j].y;
            switch (flag)
            {
            case 50:
            {
                ((cv::Mat*)*outPano2D)->ptr<Vec3b>(i)[j] = fishImgs[0].ptr<Vec3b>(y)[x];
                continue;
            }

            case 100:
            {
                ((cv::Mat*)*outPano2D)->ptr<Vec3b>(i)[j] = fishImgs[3].ptr<Vec3b>(y)[x];
                continue;
            }

            case 150:
            {
                ((cv::Mat*)*outPano2D)->ptr<Vec3b>(i)[j] = fishImgs[2].ptr<Vec3b>(y)[x];
                continue;
            }

            case 200:
            {
                ((cv::Mat*)*outPano2D)->ptr<Vec3b>(i)[j] = fishImgs[1].ptr<Vec3b>(y)[x];
                continue;
            }
            default:
                break;
            }
        }
    }

    *outSide = new Mat(fishImgs[outSideChannel]);

#endif
}

void StitchWorker::stitching_cl(const void* front, const void* rear, const void* left, const void* right,
                const cv::Mat& mapX, const cv::Mat& mapY, const cv::Mat& mask,
                void** outPano2D, int outPano2DWidth, int outPano2DHeight,
                void** outSide, int outSideWidth, int outSideHeight, int outSideChannel)
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

    *outSide = new Mat(fishImgs[outSideChannel]);
}
