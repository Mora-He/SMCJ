#pragma once

#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/core/utility.hpp"

#include <iostream>
#include <sstream>

class StereoMatch
{
public:
	cv::Rect roi1;
	cv::Rect roi2;
	cv::Mat map[2][2];

	float scale;
	int SADWindowSize;
	int numberOfDisparities;

	cv::Mat disp;
	cv::Mat depth;
	cv::Ptr<cv::StereoBM> bm;
	cv::Ptr<cv::StereoSGBM> sgbm;
	cv::Size imgSize;

	/*-----------------------------------
	 * 功能：初始化内部变量，载入双目定标结果，match参数设置
	 * 返回值：0成功，-1失败
	 *-----------------------------------
	 */
	int init();

	/*-----------------------------------
	 * 功能：匹配
	 *-----------------------------------
	 */
	int stereo_match(cv::Mat left, cv::Mat right, int x, int y, uchar* depth);
};