#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <time.h>

#include "stitch_algorithm.h"

using namespace std;
using namespace cv;


void stitching_init(const string config_path, Mat& Map, Mat& Mask);
void CarPano(const vector<Mat>& fishImgs, const Mat& Map, const Mat& Mask,  Mat** Pano2D, int Width, int Height, Mat** SideImg, int Side_Channel);

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

void stitching_init(const string config_path, Mat& Map, Mat& Mask)
{
    cout << "System Initialization:" << config_path << endl;
    FileStorage fs(config_path, FileStorage::READ);
    cout << "Reading Map" << endl;
    fs["Map"] >> Map;

    cout << "Reading Mask" << endl;
    fs["Mask"] >> Mask;
    fs.release();
    cout << ".......Initialization done......" << endl;
    return;
}

void CarPano(const vector<Mat>& fishImgs, const Mat& Map, const Mat& Mask,  Mat** Pano2D, int Width, int Height, Mat** SideImg, int Side_Channel)
{
    *Pano2D = new Mat(Height, Width, CV_8UC3);
    for (int i = 0; i < Height; i++)
    {
        for (int j = 0; j < Width; j++)
        {
            int flag = Mask.ptr<uchar>(i)[j];
            int x = Map.ptr<Point2f>(i)[j].x;
            int y = Map.ptr<Point2f>(i)[j].y;
            switch (flag)
            {
            case 50:
            {
                (*Pano2D)->ptr<Vec3b>(i)[j] = fishImgs[0].ptr<Vec3b>(y)[x];
                continue;
            }

            case 100:
            {
                (*Pano2D)->ptr<Vec3b>(i)[j] = fishImgs[3].ptr<Vec3b>(y)[x];
                continue;
            }

            case 150:
            {
                (*Pano2D)->ptr<Vec3b>(i)[j] = fishImgs[2].ptr<Vec3b>(y)[x];
                continue;
            }

            case 200:
            {
                (*Pano2D)->ptr<Vec3b>(i)[j] = fishImgs[1].ptr<Vec3b>(y)[x];
                continue;
            }
            default:
                break;
            }
        }
    }

    *SideImg = new Mat(fishImgs[Side_Channel]);

    return;
}
