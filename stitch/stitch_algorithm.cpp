#include "stitch_algorithm.h"
#include "common.h"
#include "stitch_cl.h"

using namespace std;
using namespace cv;

void CarPano(const std::vector<cv::Mat>& fishImgs,
             const cv::Mat& map, const cv::Mat& mask,
             cv::Mat** outPano2D, int outPano2DWidth, int outPano2DHeight,
             cv::Mat** outSide, int outSideWidth, int outSideHeight, int outSideChanne)
{
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
                (*outPano2D)->ptr<Vec3b>(i)[j] = fishImgs[0].ptr<Vec3b>(y)[x];
                continue;
            }

            case 100:
            {
                (*outPano2D)->ptr<Vec3b>(i)[j] = fishImgs[3].ptr<Vec3b>(y)[x];
                continue;
            }

            case 150:
            {
                (*outPano2D)->ptr<Vec3b>(i)[j] = fishImgs[2].ptr<Vec3b>(y)[x];
                continue;
            }

            case 200:
            {
                (*outPano2D)->ptr<Vec3b>(i)[j] = fishImgs[1].ptr<Vec3b>(y)[x];
                continue;
            }
            default:
                break;
            }
        }
    }

    *outSide = new Mat(fishImgs[outSideChanne]);
}

void CarPano_cl(const std::vector<cv::Mat>& fishImgs,
              const cv::Mat& mapX, const cv::Mat& mapY, const cv::Mat& mask,
              cv::Mat** outPano2D, int outPano2DWidth, int outPano2DHeight,
              cv::Mat** outSide, int outSideWidth, int outSideHeight, int outSideChannel)
{

#if CL_HELLOWORLD

    int helloworld_in[VIDEO_PANO2D_RES_Y_MAX][VIDEO_PANO2D_RES_X_MAX];
    int helloworld_out[VIDEO_PANO2D_RES_Y_MAX][VIDEO_PANO2D_RES_X_MAX];
    stitch_cl_new_helloworld_buffer(VIDEO_PANO2D_RES_X_MAX, VIDEO_PANO2D_RES_Y_MAX);
    stitch_cl_helloworld(helloworld_in, helloworld_out,VIDEO_PANO2D_RES_X_MAX, VIDEO_PANO2D_RES_Y_MAX );
    stitch_cl_delete_helloworld_buffer();
#else

    *outPano2D = new Mat(outPano2DHeight, outPano2DWidth, CV_8UC3);
    stitch_cl_2d(fishImgs, mapX, mapY, mask, **outPano2D);
#endif

    *outSide = new Mat(fishImgs[outSideChannel]);
}

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
#if CL_HELLOWORLD
        stitch_cl_init("hello_world.cl", "hello_world");
#else
        stitch_cl_init("stitch.cl", "stitch_2d");
#endif
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

    CarPano(fishImgs, map, mask, (cv::Mat**)outPano2D, outPano2DWidth, outPano2DHeight, (cv::Mat**)outSide, outSideWidth, outSideHeight, outSideChannel);

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

    CarPano_cl(fishImgs, mapX, mapY, mask, (cv::Mat**)outPano2D, outPano2DWidth, outPano2DHeight, (cv::Mat**)outSide,  outSideWidth, outSideHeight, outSideChannel);
}
