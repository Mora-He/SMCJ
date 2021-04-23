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
	 * ���ܣ���ʼ���ڲ�����������˫Ŀ��������match��������
	 * ����ֵ��0�ɹ���-1ʧ��
	 *-----------------------------------
	 */
	int init();

	/*-----------------------------------
	 * ���ܣ�ƥ��
	 *-----------------------------------
	 */
	int stereo_match(cv::Mat left, cv::Mat right, int x, int y, uchar* depth);
};