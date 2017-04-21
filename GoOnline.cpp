#include <opencv/cv.h>
#include <iostream>
#include <string>
#include <time.h>
#include "stitch_algorithm.h"
#include "common.h"

#if IMX_OPENCL
#include "stitch_cl.h"
#endif

using namespace std;
using namespace cv;

#if 0
int main()
{
    vector<Mat> srcImgs(4);
    for (int i = 0; i < 4; i++)
    {
        stringstream path;
        path << "./src/mycar6/src_" << i + 1 << ".jpg";
        srcImgs[i] = imread(path.str(), 1);
    }

    Mat Mask, Map;
    sys_init("./config3/PanoConfig.bin", Map, Mask);

    Mat Pano, Side;

    double start = clock();
    CarPano(srcImgs, Map, Mask, 3, Pano, Side);
    double end = clock();
    cout << "Time : " << (end - start) / CLOCKS_PER_SEC << endl;

    imshow("PANO", Pano);
    imshow("SideImg", Side);
    waitKey();
    return 0;
}

#endif

void stitching_init(const string config_path, Mat& map, Mat& mask)
{
    cout << "System Initialization:" << config_path << endl;
    FileStorage fs(config_path, FileStorage::READ);
    cout << "Reading Map" << endl;
    fs["Map"] >> map;

    cout << "Reading Mask" << endl;
    fs["Mask"] >> mask;
    fs.release();
    cout << ".......Initialization done......" << endl;

#if IMX_OPENCL
    stitch_cl_init("stitch.cl", "stitch_2d");
#endif

    return;
}

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

void CarPano2(const std::vector<cv::Mat>& fishImgs,
              const cv::Mat& mapX, const cv::Mat& mapY, const cv::Mat& mask,
              cv::Mat** outPano2D, int outPano2DWidth, int outPano2DHeight,
              cv::Mat** outSide, int outSideWidth, int outSideHeight, int outSideChannel)
{
#if IMX_OPENCL

    int out_image_pano2d[VIDEO_PANO2D_RES_Y][VIDEO_PANO2D_RES_X];
    stitch_cl_new_pano2d_buffer(fishImgs[0].cols, fishImgs[0].rows,
            outSideWidth, outSideHeight,
            outPano2DWidth, outPano2DHeight);
    cout << ".......stitch_cl_2d start......" << endl;
    stitch_cl_2d(fishImgs, mapX, mapY, mask, outPano2DWidth, outPano2DHeight, out_image_pano2d);
     cout << ".......stitch_cl_2d done......" << endl;
    *outPano2D = new Mat(outPano2DHeight, outPano2DWidth, CV_8UC3, out_image_pano2d);
    stitch_cl_free_pano2d_buffer();

#endif
    *outSide = new Mat(fishImgs[outSideChannel]);
}
