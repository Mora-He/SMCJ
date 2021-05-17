#include "match.h"

int StereoMatch::stereo_match(cv::Mat leftFrame, cv::Mat rightFrame, int x, int y, uchar* depthData)
{
	cv::Mat left, right;
	cv::Mat leftTemp, rightTemp;
	cv::Mat dispTemp;

	if (leftFrame.empty() || rightFrame.empty())
	{
		std::cout << "error: img empty" << std::endl;
		return -1;
	}

	if (scale != 1.f)
	{
		int method = scale < 1 ? cv::INTER_AREA : cv::INTER_CUBIC;
		resize(leftFrame, left, cv::Size(), scale, scale, method);
		resize(rightFrame, right, cv::Size(), scale, scale, method);
	}
	else
	{
		left = leftFrame;
		right = rightFrame;
	}
	cv::remap(left, leftTemp, map[0][0], map[0][1], cv::INTER_LINEAR);
	cv::remap(right, rightTemp, map[1][0], map[1][1], cv::INTER_LINEAR);
	cv::cvtColor(leftTemp, left, cv::COLOR_BGR2GRAY);
	cv::cvtColor(rightTemp, right, cv::COLOR_BGR2GRAY);	

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
		int method = scale < 1 ? cv::INTER_AREA : cv::INTER_CUBIC;
		cv::resize(disp8, disp, cv::Size(), 1 / scale, 1 / scale, method);
	}
	else
	{
		disp8.copyTo(disp);
	}

	uchar* dispData = (uchar*)disp.data;
	int id = x * disp.cols + y;

	if (!dispData[id]) 
		*depthData = 0;
	else
	{
		*depthData = uchar((float)fxMulBase / (float)dispData[id]);
	}

	//cv::rectangle(leftTemp, roi1, cv::Scalar(0, 0, 255), 3, 8);

	//cv::Mat leftTempBig;
	//int method = scale > 1 ? cv::INTER_AREA : cv::INTER_CUBIC;
	//cv::resize(leftTemp, leftTempBig, cv::Size(), scale, scale, method);

	//cv::imshow("rec", leftTempBig);
	//cv::Mat disp8_3c;
	//cv::applyColorMap(disp, disp8_3c, cv::COLORMAP_TURBO);
	//cv::imshow("disp8_3c", disp8_3c);
	//cv::waitKey();

	int width = disp.cols;
	int height = disp.rows;
	cv::Mat depth(width, height, CV_8UC1);
	uchar* depth_data = (uchar*)depth.data;
	int min_data = 205;
	int sx = 0, sy = 0;
	int ex = 0,ey = 0;

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
			//std::cout << (int)depth_data[id] << " ";
			if (depth_data[id] < min_data)
			{
				min_data = depth_data[id];
				sx = m;
				sy = n;
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
	std::cout << min_data << std::endl;
	//int can = min_data + 1;
	//for (int m = 0; m < sx; m++)
	//{
	//	int wi = m * width;
	//	for (int n = 0; n < width; n++)
	//	{
	//		if (depth_data[wi + n] == can)
	//		{
	//			if (sy > n) sy = n;
	//			if (ey < n) ey = n;
	//			sx = m;
	//		}
	//	}
	//}

	//int max_dis = dispData[std::max_element(dispData, dispData + disp.cols*disp.rows) - dispData];
	//std::cout << ((float)fxMulBase / max_dis) << std::endl;
	//int min_location = std::max_element(dispData, dispData + disp.cols*disp.rows) - dispData;
	//int sx = min_location / width;
	//int sy = min_location % width;
	//int min_data = depth_data[min_location];
	//int ex = sx, ey;
	//int ix = sx;
	//int i = sx, j;

	//// 初始化
	//int iwidth = i * width;
	//for (j = sy + 1; j < width; j++)
	//{
	//	//for (int m = 0; m < width; m++)
	//	//{
	//	//	//std::cout << (int)depth_data[iwidth + m] << " ";
	//	//	if ((int)depth_data[iwidth + m] == min_data) 
	//	//		std::cout << m << std::endl;
	//	//}
	//	if (depth_data[iwidth + j] != min_data) break;
	//}
	//ey = j - 1;

	//// 每一行
	//for (i = sx + 1; i < height; i++)
	//{
	//	iwidth = i * width;
	//	for (j = 0; j < width; j++)
	//	{

	//	}
		//for (int m = 0; m < width; m++)
		//{
		//	std::cout << (int)depth_data[iwidth + m] << " ";
		//}
		//std::cout << std::endl;

		//// 左边扩展
		//if (depth_data[iwidth + sy] == min_data)
		//{
		//	for (j = sy - 1; j >= 0; j--)
		//	{
		//		if (depth_data[iwidth + j] != min_data) break;
		//	}
		//	sy = j + 1;
		//}
		//else
		//{
		//	for (j = sy + 1; j < width; j++)
		//	{
		//		if (depth_data[iwidth + j] == min_data) break;
		//	}
		//	if (depth_data[iwidth + j] != min_data) break;
		//}
		//// 左边回缩
		//else
		//{
		//	for (j = left_iy + 1; j < ey; j++)
		//	{
		//		if (depth_data[iwidth + j] == min_data) break;
		//	}
		//	if (depth_data[iwidth + j] == min_data)
		//	{
		//		left_iy = j;
		//	}
		//	else {
		//		break;
		//	}
		//}
		// 右边扩展
		//if (depth_data[iwidth + ey] == min_data)
		//{
		//	for (j = ey + 1; j < width; j++)
		//	{
		//		if (depth_data[iwidth + j] != min_data) break;
		//	}
		//	ey = j - 1;
		//}
		// 右边回缩
		//else
		//{
		//	for (j = ey - 1; j > sy; j--)
		//	{
		//		if (depth_data[iwidth + j] == min_data) break;
		//	}
		//	if (depth_data[iwidth + j] == min_data)
		//	{
		//		right_iy = j;
		//	}
		//	else {
		//		break;
		//	}
		//}
		//// 调整sy，ey
		//if (left_iy < sy)
		//{
		//	sy = left_iy;
		//}
		//if (right_iy > ey)
		//{
		//	ey = right_iy;
		//}
	//}
	//ex = i - 1;


	// ！！！！！！！！！！！！！！！！！！！
	std::cout << sx << " " << sy << " " << ex << " " << ey << std::endl;
	cv::rectangle(leftFrame, cv::Rect(sy,sx,ey-sy,ex-sx), cv::Scalar(0, 0, 255), 3, 8);
	cv::imshow("left", leftFrame);
	cv::waitKey();
	//for (int i = 0; i < maxBottom; i+=length)
	//{
	//	for (int j = 0; j < maxLeft; j+=length)
	//	{
	//		int start = i * width + j;
	//		int sum = 0;
	//		for (int m = 0; m < length; m++)
	//		{
	//			int wi = m * width + start;
	//			for (int n = 0; n < length; n++)
	//			{
	//				int id = wi + n;
	//				if (!dispData[id]) continue;
	//				depth.data[id] = uchar((float)fxMulBase / (float)dispData[id]);
	//				sum += depth_data[id];
	//			}
	//		}
	//		std::cout << sum / pow << " ";
	//	}
	//	std::cout << std::endl;
	//}
	//for (int i = 0; i < maxBottom; i += length)
	//{
	//	for (int j = 0; j < maxLeft; j += length)
	//	{
	//		int start_x = roi1.x / scale + i;
	//		int start_y = roi1.y / scale + j;
	//		int start = start_x * width + start_y;
	//		int sum = 0;
	//		for (int k = 0; k < length; k++)
	//		{
	//			int wi = k * width;
	//			for (int m = 0; m < length; m++)
	//			{
	//				sum += dispData[start + wi + m];
	//			}
	//		}
	//		if (sum / pow > max)
	//		{
	//			max_x = start_x;
	//			max_y = start_y;
	//			max = sum / pow;
	//		}
	//		std::cout << sum / pow << " ";
	//	}
	//	std::cout << std::endl;
	//}

	return 0;
}

int StereoMatch::init()
{
	std::string filename = "intrinsic.yml";
	cv::Mat M1, D1, M2, D2;
	cv::Mat R, T, R1, P1, R2, P2, Q;
	int cn = 3;			// 帧通道数3

	SADWindowSize = 9;
<<<<<<< HEAD
	scale = 1;							// 调参参数更改！！！！！！！！！！	1 0.6 923； 0 0.2/0.4
	imgSize = cv::Size(640*scale, 480*scale);		// 图片像素大小不能改变，如果图片改变，更改像素大小
=======
	scale = 0.7;							// ���β������ģ�������������������	1 0.6 923�� 0 0.2/0.4
	imgSize = cv::Size(640*scale, 480*scale);		// ͼƬ���ش�С���ܸı䣬���ͼƬ�ı䣬�������ش�С
>>>>>>> ddd4e6670797755283b14aa8cde4f25ea6ae6498
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
	//M1.create(cv::Size(3, 3), CV_64FC1);
	//D1.create(cv::Size(14, 1), CV_64FC1);
	//M2.create(cv::Size(3, 3), CV_64FC1);
	//D2.create(cv::Size(14, 1), CV_64FC1);
	//R.create(cv::Size(3, 3), CV_64FC1);
	//T.create(cv::Size(1, 3), CV_64FC1);

	//std::vector<double> M1_data = { 2.5820512735289833e+02, 0., 3.1950000000000000e+02, 0.,
	//   3.4406783803722487e+02, 2.3950000000000000e+02, 0., 0., 1. };
	//std::vector<double> D1_data = { -3.2544921217963700e-02, 8.6735853040987826e-02,
	//   -2.2932361872953903e-03, -1.1094704196729626e-02, 0., 0., 0., 0.,
	//   0., 0., 0., 0., 0., 0. };
	//std::vector<double> M2_data = { 2.5820512735289833e+02, 0., 3.1950000000000000e+02, 0.,
	//   3.4406783803722487e+02, 2.3950000000000000e+02, 0., 0., 1. };
	//std::vector<double> D2_data = { 5.9463308800314467e-03, 8.3572304929435293e-02,
	//   -1.6389790977124161e-02, 1.2055760066227937e-02, 0., 0., 0., 0.,
	//   0., 0., 0., 0., 0., 0. };
	//std::vector<double> R_data = { 9.9998573822416781e-01, -4.7891047510422126e-03,
	//   -2.3638578530869375e-03, 4.9164961997599796e-03,
	//   9.9835042408239283e-01, 5.7203660718734922e-02,
	//   2.0860041667747336e-03, -5.7214466791100455e-02,
	//   9.9835973144775136e-01 };
	//std::vector<double> T_data = { -3.1235343663935828e+00, -3.1518016128995331e-02,
	//   -2.8709420691250243e-01 };


	//cv::Mat M1_temp = cv::Mat(M1_data);
	//M1 = M1_temp.reshape(1, 3).clone();
	//cv::Mat D1_temp = cv::Mat(D1_data);
	//D1 = D1_temp.reshape(1, 1).clone();
	//cv::Mat M2_temp = cv::Mat(M2_data);
	//M2 = M2_temp.reshape(1, 3).clone();
	//cv::Mat D2_temp = cv::Mat(D2_data);
	//D2 = D2_temp.reshape(1, 1).clone();
	//cv::Mat R_temp = cv::Mat(R_data);
	//R = R_temp.reshape(1, 3).clone();
	//cv::Mat T_temp = cv::Mat(T_data);
	//T = T_temp.reshape(1, 3).clone();

	M1 *= scale;
	M2 *= scale;

	fs["R"] >> R;
	fs["T"] >> T;

	cv::stereoRectify(M1, D1, M2, D2, imgSize, R, T, R1, R2, P1, P2, Q, cv::CALIB_ZERO_DISPARITY, 1, imgSize, &roi1, &roi2);

	cv::initUndistortRectifyMap(M1, D1, R1, P1, imgSize, CV_16SC2, map[0][0], map[0][1]);
	cv::initUndistortRectifyMap(M2, D2, R2, P2, imgSize, CV_16SC2, map[1][0], map[1][1]);

	//cv::FileStorage fm("map.yml", cv::FileStorage::WRITE);
	//fm << "map00" << map[0][0];
	//fm << "map01" << map[0][1];
	//fm << "map10" << map[1][0];
	//fm << "map11" << map[1][1];
	cv::FileStorage fm("map.yml", cv::FileStorage::READ);
	fm["map00"] >> map[0][0];
	fm["map01"] >> map[0][1];
	fm["map10"] >> map[1][0];
	fm["map11"] >> map[1][1];


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