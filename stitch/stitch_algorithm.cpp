#include "stitch_algorithm.h"
#include "common.h"
#include "clpano2d.h"

using namespace std;
using namespace cv;

CLPano2D gCLPano2D;

void stitching_init(const string config_path, Mat& map, Mat& mask, bool enableOpenCL)
{
    cout << "System Initialization:" << config_path << endl;
    FileStorage fs(config_path, FileStorage::READ);
    cout << "Reading Map" << endl;
    fs["Map"] >> map;

    cout << "Reading Mask" << endl;
    fs["Mask"] >> mask;
    fs.release();
    cout << ".......Initialization done......" << endl;

    if (enableOpenCL)
    {
        gCLPano2D.init("stitch.cl", "stitch_2d");
    }

    return;
}

void stitching(const void* front, const void* rear, const void* left, const void* right,
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

void stitching_cl(const void* front, const void* rear, const void* left, const void* right,
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
    gCLPano2D.stitch_cl_2d(fishImgs, mapX, mapY, mask, *((cv::Mat*)*outPano2D));

    *outSide = new Mat(fishImgs[outSideChannel]);
}
