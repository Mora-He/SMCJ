#pragma once

#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/core/utility.hpp"

#include <iostream>
#include <sstream>
#include <numeric>

class StereoMatch
{
public:
	cv::Rect roi1;
	cv::Rect roi2;
	cv::Mat map[2][2];

	float scale;
	int SADWindowSize;
	int numberOfDisparities;

	cv::Mat disp, disp8;
	cv::Ptr<cv::StereoBM> bm;
	cv::Ptr<cv::StereoSGBM> sgbm;
	cv::Size imgSize;

	float fxMulBase;

	int alg;	// 0:BM, 1:SGBM;

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
	int stereo_match(cv::Mat leftFrame, cv::Mat rightFrame, int x, int y, uchar* depthData);
};