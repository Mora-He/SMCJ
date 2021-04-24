#include "match.hpp"

int StereoMatch::stereo_match(cv::Mat leftFrame, cv::Mat rightFrame, int x, int y, uchar* depthData)
{
	// cv::Mat left, right;
	// cv::Mat leftTemp, rightTemp;
	// cv::Mat dispTemp;

	// if (leftFrame.empty() || rightFrame.empty())
	// {
	// 	std::cout << "error: img empty" << std::endl;
	// 	return -1;
	// }

	// if (scale != 1.f)
	// {
	// 	int method = scale < 1 ? cv::INTER_AREA : cv::INTER_CUBIC;
	// 	resize(leftFrame, left, cv::Size(), scale, scale, method);
	// 	resize(rightFrame, right, cv::Size(), scale, scale, method);
	// }
	// else
	// {
	// 	left = leftFrame;
	// 	right = rightFrame;
	// }
	// remap(left, leftTemp, map[0][0], map[0][1], cv::INTER_LINEAR);
	// remap(right, rightTemp, map[1][0], map[1][1], cv::INTER_LINEAR);
	// cv::cvtColor(leftTemp, left, cv::COLOR_BGR2GRAY);
	// cv::cvtColor(rightTemp, right, cv::COLOR_BGR2GRAY);	

	// // bm
	// if (alg == 0)
	// {
	// 	bm->compute(left, right, dispTemp);
	// }
	// // sgbm
	// else
	// {
	// 	sgbm->compute(left, right, dispTemp);
	// }

	// dispTemp.convertTo(disp8, CV_8U, 255 / (numberOfDisparities * 16.));
	

	// scale = 1 / scale;
	// if (scale != 1.f)
	// {
	// 	int method = scale > 1 ? cv::INTER_AREA : cv::INTER_CUBIC;
	// 	cv::resize(disp8, disp, cv::Size(), scale, scale, method);
	// }
	// else
	// {
	// 	disp8.copyTo(disp);
	// }

	// uchar* dispData = (uchar*)disp.data;
	// int id = x * disp.cols + y;

	// if (!dispData[id]) 
	// 	*depthData = 0;
	// else
	// {
	// 	*depthData = uchar((float)fxMulBase / (float)dispData[id]);
	// }
	return 0;
}

int StereoMatch::init()
{
	// std::string filename = "/mnt/app/intrinsic.yml";
	char filename[23] = "/mnt/app/intrinsic.yml";
	cv::Mat M1, D1, M2, D2;
	cv::Mat R, T, R1, P1, R2, P2, Q;
	int cn = 3;			// 帧通道数3

	SADWindowSize = 9;
	scale = 1;							// 调参参数更改！！！！！！！！！！
	imgSize = cv::Size(640, 480);		// 图片像素大小不能改变，如果图片改变，更改像素大小
	numberOfDisparities = ((imgSize.width / 8) + 15) & -16;
	alg = 0;

	bm = cv::StereoBM::create(16, 3);
	sgbm = cv::StereoSGBM::create(0, 16, 3);

	cv::FileStorage fs(filename, cv::FileStorage::READ);
	if (!fs.isOpened())
	{
		return -1;
	}

	fs["M1"] >> M1;
	fs["D1"] >> D1;
	fs["M2"] >> M2;
	fs["D2"] >> D2;

	M1 *= scale;
	M2 *= scale;

	fs["R"] >> R;
	fs["T"] >> T;

	cv::stereoRectify(M1, D1, M2, D2, imgSize, R, T, R1, R2, P1, P2, Q, cv::CALIB_ZERO_DISPARITY, 1, imgSize, &roi1, &roi2);

	cv::initUndistortRectifyMap(M1, D1, R1, P1, imgSize, CV_16SC2, map[0][0], map[0][1]);
	cv::initUndistortRectifyMap(M2, D2, R2, P2, imgSize, CV_16SC2, map[1][0], map[1][1]);

	// bm
	bm->setROI1(roi1);
	bm->setROI2(roi2);
	bm->setPreFilterCap(31);
	bm->setBlockSize(SADWindowSize);
	bm->setMinDisparity(0);
	bm->setNumDisparities(numberOfDisparities);
	bm->setTextureThreshold(10);
	bm->setUniquenessRatio(15);
	bm->setSpeckleWindowSize(100);
	bm->setSpeckleRange(32);
	bm->setDisp12MaxDiff(1);

	// sgbm
	sgbm->setPreFilterCap(32);
	sgbm->setBlockSize(SADWindowSize);

	int temp = cn * SADWindowSize * SADWindowSize;
	sgbm->setP1(8 * temp);
	sgbm->setP2(32 * temp);
	sgbm->setMinDisparity(0);
	sgbm->setNumDisparities(numberOfDisparities);
	sgbm->setUniquenessRatio(10);
	sgbm->setSpeckleWindowSize(100);
	sgbm->setSpeckleRange(32);
	sgbm->setDisp12MaxDiff(1);
	sgbm->setMode(cv::StereoSGBM::MODE_SGBM);

	// 转深度图
	float fx = (float)(M1.at<double>(0, 0) + M2.at<double>(0, 0)) / 2;
	float baseline = sqrt((float)T.at<double>(0, 0)*T.at<double>(0, 0)
		+ T.at<double>(1, 0)*T.at<double>(1, 0)
		+ T.at<double>(2, 0)*T.at<double>(2, 0));
	fxMulBase = fx * baseline;

	return 0;
}