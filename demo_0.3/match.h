#pragma once

#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/core/utility.hpp"

#include <iostream>
#include <sstream>

using namespace cv;
using namespace std;

enum
{
	STEREO_BM = 0,
	STEREO_SGBM = 1,
	STEREO_HH = 2,
	STEREO_VAR = 3,
	STEREO_3WAY = 4,
	STEREO_HH4 = 5
};

int stereo_match(Mat leftFrame, Mat rightFrame, Mat* depth, int alg, bool outFlag, bool speFlag, bool fileFlag);
static void saveXYZ(const char* filename, const Mat& mat);