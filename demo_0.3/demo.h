#pragma once

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <array>
#include <map>
#include <iostream>
#include "calib.h"
#include "match.h"
#include <cmath>

using namespace std;
using namespace cv;

enum {
	NONE = 0,
	RECTIFY = 1,
	DISTANCE = 2,
	LENGTH = 3
};

void empty_img_init(String text, Mat* emptyImg);
void disp_to_depth(Mat disp, Mat* depth);