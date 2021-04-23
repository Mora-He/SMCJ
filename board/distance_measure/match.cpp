#include "match.hpp"

int StereoMatch::stereo_match(cv::Mat left, cv::Mat right, int x, int y, uchar* depth)
{
	return 0;
}

int StereoMatch::init()
{
	// std::string filename = "intrinsic.yml";
	// cv::Mat M1, D1, M2, D2;
	// cv::Mat R, T, R1, P1, R2, P2, Q;

	// SADWindowSize = 9;
	// scale = 0.5;
	// imgSize = cv::Size(640, 480);

	// bm = cv::StereoBM::create(16, 3);
	// sgbm = cv::StereoSGBM::create(0, 16, 3);

	// cv::FileStorage fs(filename, cv::FileStorage::READ);
	// if (!fs.isOpened())
	// {
	// 	return -1;
	// }

	// fs["M1"] >> M1;
	// fs["D1"] >> D1;
	// fs["M2"] >> M2;
	// fs["D2"] >> D2;

	// M1 *= scale;
	// M2 *= scale;

	// fs["R"] >> R;
	// fs["T"] >> T;

	// cv::stereoRectify(M1, D1, M2, D2, imgSize, R, T, R1, R2, P1, P2, Q, cv::CALIB_ZERO_DISPARITY, 1, imgSize, &roi1, &roi2);

	// cv::initUndistortRectifyMap(M1, D1, R1, P1, imgSize, CV_16SC2, map[0][0], map[0][1]);
	// cv::initUndistortRectifyMap(M2, D2, R2, P2, imgSize, CV_16SC2, map[1][0], map[1][1]);

	return 0;
}