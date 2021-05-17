#include "match.hpp"

// int StereoMatch::stereo_match(cv::Mat leftFrame, cv::Mat rightFrame, int x, int y, uchar* depthData)
int StereoMatch::stereo_match(cv::Mat leftFrame, cv::Mat rightFrame)
{
	cv::Mat left, right;
	cv::Mat leftTemp, rightTemp;
	cv::Mat leftSmall, rightSmall;
	cv::Mat dispTemp;
	
	// printf("mark6\n");
	if (leftFrame.empty() || rightFrame.empty())
	{
		std::cout << "error: img empty" << std::endl;
		return -1;
	}

	if (scale != 1.f)
	{
		int method = scale < 1 ? cv::INTER_AREA : cv::INTER_CUBIC;
		resize(leftFrame, leftSmall, cv::Size(), scale, scale, method);
		resize(rightFrame, rightSmall, cv::Size(), scale, scale, method);
	}
	else
	{
		leftFrame.copyTo(leftSmall);
		rightFrame.copyTo(rightSmall);
	}
	// printf("mark7\n");
	// cv::remap(left, leftTemp, map[0][0], map[0][1], cv::INTER_LINEAR);
	cv::remap(leftSmall, leftTemp, map00, map01, cv::INTER_LINEAR);
	// cv::remap(right, rightTemp, map[1][0], map[1][1], cv::INTER_LINEAR);
	cv::remap(rightSmall, rightTemp, map10, map11, cv::INTER_LINEAR);
	// printf("mark8\n");
	cv::cvtColor(leftTemp, left, cv::COLOR_BGR2GRAY);
	cv::cvtColor(rightTemp, right, cv::COLOR_BGR2GRAY);	
// printf("mark9\n");
	// bm
	if (alg == 0)
	{
		bm->compute(left, right, dispTemp);
	}
	// sgbm
	else
	{
		sgbm->compute(left, right, dispTemp);
	}

	dispTemp.convertTo(disp8, CV_8U, 255 / (numberOfDisparities * 16.));
	

	if (scale != 1.f)
	{
		int method = 1 / scale < 1 ? cv::INTER_AREA : cv::INTER_CUBIC;
		cv::resize(disp8, disp, cv::Size(), (float)1 / scale, (float)1 / scale, method);
	}
	else
	{
		disp8.copyTo(disp);
	}

	uchar* dispData = (uchar*)disp.data;
	int id = 0;

	// if (!dispData[id]) 
	// 	*depthData = 0;
	// else
	// {
	// 	*depthData = uchar((float)fxMulBase / (float)dispData[id]);
	// }

//////////////////////////////////////////////////
	int width = disp.cols;
	int height = disp.rows;
	cv::Mat depth(width, height, CV_8UC1);
	uchar* depth_data = (uchar*)depth.data;
	int min_data = 205;
	sx = 0, sy = 0;
	ex = 0, ey = 0;

	for (int m = 0; m < height; m++)
	{
		int wi = m * width;
		for (int n = 0; n < width; n++)
		{
			id = wi + n;
			if (!dispData[id])
			{
				//std::cout << (int)depth_data[id] << " ";
				continue;
			}
			depth_data[id] = uchar(cvRound((float)fxMulBase / ((float)dispData[id] * scale)));
			if(depth_data[id] == 0) continue;
			//std::cout << (int)depth_data[id] << " ";
			if (depth_data[id] < min_data)
			{
				min_data = depth_data[id];
				sx = m;
				sy = n;
				ex = m;
				ey = n;
			}
			else if (depth_data[id] == min_data)
			{
				if (sy > n) sy = n;
				if (ey < n) ey = n;
				ex = m;
			}
		}
		//std::cout << std::endl;
	}
	// std::cout << "深度图的最小距离为：" << min_data << "分米" << std::endl;
	
	// std::cout << sx << " " << sy << " " << ex << " " << ey << std::endl;
	if(count < 60000)
		count++;
	else
		count = 0;
	// std::string dept_path = "/mnt/app/photos/depth";
	
	// if(count%10 == 0)
	// {
	// 	cv::FileStorage depth_fs("/mnt/app/photo.yml", cv::FileStorage::APPEND);
	// 	// depth_fs << "depth" + std::to_string(count) << depth;
	// 	depth_fs << "left" + std::to_string(count) << leftFrame;
	// 	depth_fs << "right" + std::to_string(count) << rightFrame;
	// 	std::cout<< count << std::endl;
	// 	depth_fs.release();
	// }
	// ex += 10;
	// ey += 10;
	return min_data;
}

int StereoMatch::init()
{
	cv::Mat M1, D1, M2, D2;
	cv::Mat R, T, R1, P1, R2, P2, Q;
	int cn = 3;			// 帧通道数3

	SADWindowSize = 9;
	scale = 0.7;							// 调参参数更改！！！！！！！！！！
	imgSize = cv::Size(640*scale, 480*scale);		// 图片像素大小不能改变，如果图片改变，更改像素大小
	numberOfDisparities = ((imgSize.width / 8) + 15) & -16;
	alg = 0;

	bm = cv::StereoBM::create(16, 3);
	sgbm = cv::StereoSGBM::create(0, 16, 3);
// printf("mark1\n");
	cv::FileStorage fs("/mnt/app/intrinsic.yml", cv::FileStorage::READ);
	if (!fs.isOpened())
	{
		printf("cannot open\n");
		return -1;
	}
// printf("mark2\n");
	fs["M1"] >> M1;
	fs["D1"] >> D1;
	fs["M2"] >> M2;
	fs["D2"] >> D2;
	fs["R"] >> R;
	fs["T"] >> T;

	
	M1 *= scale;
	M2 *= scale;

	cv::stereoRectify(M1, D1, M2, D2, imgSize, R, T, R1, R2, P1, P2, Q, cv::CALIB_ZERO_DISPARITY, 1, imgSize, &roi1, &roi2);

	// cv::initUndistortRectifyMap(M1, D1, R1, P1, imgSize, CV_16SC2, map[0][0], map[0][1]);
	cv::initUndistortRectifyMap(M1, D1, R1, P1, imgSize, CV_16SC2, map00, map01);

	// cv::initUndistortRectifyMap(M2, D2, R2, P2, imgSize, CV_16SC2, map[1][0], map[1][1]);
	cv::initUndistortRectifyMap(M2, D2, R2, P2, imgSize, CV_16SC2, map10, map11);

	// cv::FileStorage fm("/mnt/app/map.yml", cv::FileStorage::READ);
	// fm["map00"] >> map00;
	// fm["map01"] >> map01;
	// fm["map10"] >> map10;
	// fm["map11"] >> map11;
// printf("mark3\n");
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
// printf("mark4\n");
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
// printf("mark5\n");
	// 转深度图
	float fx = (float)(M1.at<double>(0, 0) + M2.at<double>(0, 0)) / 2;
	float baseline = sqrt((float)T.at<double>(0, 0)*T.at<double>(0, 0)
		+ T.at<double>(1, 0)*T.at<double>(1, 0)
		+ T.at<double>(2, 0)*T.at<double>(2, 0));
	fxMulBase = fx * baseline;

	sx = 0, sy = 0;
	ex = 0, ey = 0;
	count = 0;
	return 0;
}