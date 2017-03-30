#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <time.h>

#include "stitch_algorithm.h"

using namespace std;
using namespace cv;


void stitching_init(const string config_path, vector<Mat>& Maps);
void CarPano(const vector<Mat>& fishImgs, const vector<Mat>& Maps, const int Channel, Mat** Pano2D, Mat** SideImg);
static Mat MaxFusion(const Mat img1, const Mat img2);

#if 0
int main()
{
	vector<Mat> Maps;
    stitching_init("./config2/Fish2Pano.xml", Maps);

	vector<Mat> srcImgs(4);
	for (int i = 0; i < 4; i++)
	{
		stringstream path;
		path << "./src/mycar5/src (" << i + 1 << ").jpg";
		srcImgs[i] = imread(path.str(), 1);
	}

	Mat Pano2D, SideImg;

	double start = clock();
	CarPano(srcImgs, Maps, 4, Pano2D, SideImg);////////////////////////////////////////////////
	double end = clock();
	cout << "Time to 2D Pano: " << (end - start) / CLOCKS_PER_SEC << endl;
	
	imshow("Pano2D", Pano2D);
	imshow("Channel = 1", SideImg);
	waitKey(0);
	return 0;
}

#endif

void stitching_init(const string config_path, vector<Mat>& Maps)
{
	cout << "System Initialization" << endl;
	FileStorage fs(config_path, FileStorage::READ);

	cout << "Front" << endl;
	for (int i = 1; i <= 3; i++)
	{
		Mat tmp;
		stringstream lable;
		lable << "Front_" << i;
		fs[lable.str()] >> tmp;
		Maps.push_back(tmp);
	}

	cout << "Back" << endl;
	for (int i = 1; i <= 3; i++)
	{
		Mat tmp;
		stringstream lable;
		lable << "Back_" << i;
		fs[lable.str()] >> tmp;
		Maps.push_back(tmp);
	}

	cout << "Left" << endl;
	for (int i = 1; i <= 3; i++)
	{
		Mat tmp;
		stringstream lable;
		lable << "Left_" << i;
		fs[lable.str()] >> tmp;
		Maps.push_back(tmp);
	}

	cout << "Right" << endl;
	for (int i = 1; i <= 3; i++)
	{
		Mat tmp;
		stringstream lable;
		lable << "Right_" << i;
		fs[lable.str()] >> tmp;
		Maps.push_back(tmp);
	}
	fs.release();
	cout << ".......done......" << endl;
}

#define FAST 1
#if FAST

void CarPano(const vector<Mat>& fishImgs, const vector<Mat>& Maps, const int Channel, Mat** Pano2D, Mat** SideImg)
{
    Mat fusion_res;
    vector< vector<Mat> > comp(3, vector<Mat>(3));

    remap(fishImgs[0], comp[0][1], Maps[1], Mat(), INTER_NEAREST);
    remap(fishImgs[1], comp[2][1], Maps[4], Mat(), INTER_NEAREST);
    remap(fishImgs[2], comp[1][0], Maps[7], Mat(), INTER_NEAREST);
    remap(fishImgs[3], comp[1][2], Maps[10], Mat(), INTER_NEAREST);
    comp[1][1] = Mat::zeros(comp[1][0].rows, comp[0][1].cols, fishImgs[0].type());

    remap(fishImgs[2], comp[0][0], Maps[8], Mat(), INTER_NEAREST);

    remap(fishImgs[3], comp[0][2], Maps[9], Mat(), INTER_NEAREST);

    remap(fishImgs[2], comp[2][0], Maps[6], Mat(), INTER_NEAREST);

    remap(fishImgs[3], comp[2][2], Maps[11], Mat(), INTER_NEAREST);

    vector<Mat> horizens(3);
    for (int i = 0; i < 3; i++)
    {
        hconcat(comp[i], horizens[i]);
    }
    vconcat(horizens, fusion_res);

    *SideImg = new Mat(fishImgs[Channel]);
    *Pano2D = new Mat(fusion_res);
    return;
}

#else

void CarPano(const vector<Mat>& fishImgs, const vector<Mat>& Maps, const int Channel, Mat** Pano2D, Mat** SideImg)
{
    Mat fusion1, fusion2, fusion_res;
    vector< vector<Mat> > comp(3, vector<Mat>(3));

    remap(fishImgs[0], comp[0][1], Maps[1], Mat(), INTER_LINEAR);

    remap(fishImgs[1], comp[2][1], Maps[4], Mat(), INTER_LINEAR);

    remap(fishImgs[2], comp[1][0], Maps[7], Mat(), INTER_LINEAR);

    remap(fishImgs[3], comp[1][2], Maps[10], Mat(), INTER_LINEAR);
    comp[1][1] = Mat::zeros(comp[1][0].rows, comp[0][1].cols, fishImgs[0].type());

    remap(fishImgs[0], fusion1, Maps[0], Mat(), INTER_LINEAR);
    remap(fishImgs[2], fusion2, Maps[8], Mat(), INTER_LINEAR);
    addWeighted(fusion1, 0.5, fusion2, 0.5, 0, comp[0][0]);
    //comp[0][0] = MaxFusion(fusion1, fusion2);

    remap(fishImgs[0], fusion1, Maps[2], Mat(), INTER_LINEAR);
    remap(fishImgs[3], fusion2, Maps[9], Mat(), INTER_LINEAR);
    addWeighted(fusion1, 0.5, fusion2, 0.5, 0, comp[0][2]);
    //comp[0][2] = MaxFusion(fusion1, fusion2);

    remap(fishImgs[1], fusion1, Maps[5], Mat(), INTER_LINEAR);
    remap(fishImgs[2], fusion2, Maps[6], Mat(), INTER_LINEAR);
    addWeighted(fusion1, 0.5, fusion2, 0.5, 0, comp[2][0]);
    //comp[2][0] = MaxFusion(fusion1, fusion2);

    remap(fishImgs[1], fusion1, Maps[3], Mat(), INTER_LINEAR);
    remap(fishImgs[3], fusion2, Maps[11], Mat(), INTER_LINEAR);
    addWeighted(fusion1, 0.5, fusion2, 0.5, 0, comp[2][2]);
    //comp[2][2] = MaxFusion(fusion1, fusion2);

    vector<Mat> horizens(3);
    for (int i = 0; i < 3; i++)
    {
        hconcat(comp[i], horizens[i]);
    }
    vconcat(horizens, fusion_res);

    *SideImg = new Mat(fishImgs[Channel]);
    *Pano2D = new Mat(fusion_res);
    return;
}

#endif

static Mat MaxFusion(const Mat img1, const Mat img2)
{
	int row = img1.rows;
	int col = img1.cols;
	Mat tmp(row, col, img1.type());

	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			int sum1 = img1.ptr<Vec3b>(i)[j][0] + img1.ptr<Vec3b>(i)[j][1] + img1.ptr<Vec3b>(i)[j][2];
			int sum2 = img2.ptr<Vec3b>(i)[j][0] + img2.ptr<Vec3b>(i)[j][1] + img2.ptr<Vec3b>(i)[j][2];
			tmp.ptr<Vec3b>(i)[j] = sum1>sum2 ? img1.ptr<Vec3b>(i)[j] : img2.ptr<Vec3b>(i)[j];
		}
	}
	return tmp;
}
