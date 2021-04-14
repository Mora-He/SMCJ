#include "match.h"

static void saveXYZ(const char* filename, const Mat& mat)
{
	const double max_z = 1.0e4;
	FILE* fp;
	fopen_s(&fp, filename, "wt");
	for (int y = 0; y < mat.rows; y++)
	{
		for (int x = 0; x < mat.cols; x++)
		{
			Vec3f point = mat.at<Vec3f>(y, x);
			if (fabs(point[2] - max_z) < FLT_EPSILON || fabs(point[2]) > max_z) continue;
			fprintf(fp, "%f %f %f\n", point[0], point[1], point[2]);
		}
	}
	fclose(fp);
}

int stereo_match(Mat leftFrame, Mat rightFrame, Mat* depth, int alg, bool outFlag, bool speFlag, bool fileFlag)
{
	Mat left = leftFrame, right = rightFrame, disparity;
	int SADWindowSize = 9, numberOfDisparities = -1;		// SAD窗口大小（匹配块大小），最大视差(原256)！！！！！！！！！！！
	bool no_display = true, color_display = false;
	float scale = 0.3;
	string intrinsic_filename = speFlag ? "intrinsic.yml" : "";	// 输入
	string extrinsic_filename = speFlag ? "extrinsic.yml" : "";
	string disparity_filename = fileFlag ? "disparity.jpg" : "";	// 视差图文件
	string point_cloud_filename = fileFlag ? "point_cloud.pcd" : "";	// 点云图文件

	Ptr<StereoBM> bm = StereoBM::create(16, 9);
	Ptr<StereoSGBM> sgbm = StereoSGBM::create(0, 16, 3);

	// 判空
	if (leftFrame.empty() || rightFrame.empty())
	{
		cout << "error:img empty" << endl;
		return -1;
	}

	// 图片缩放
	if (scale != 1.f)
	{
		int method = scale < 1 ? INTER_AREA : INTER_CUBIC;
		resize(leftFrame, left, Size(), scale, scale, method);
		resize(rightFrame, right, Size(), scale, scale, method);
	}

	//cvtColor(leftFrame, left, cv::COLOR_RGB2GRAY);	// test
	//cvtColor(rightFrame, right, cv::COLOR_RGB2GRAY);

	Size imgSize = left.size();
	Rect roi1, roi2;
	Mat Q;

	// 畸变矫正
	if (!intrinsic_filename.empty() && !extrinsic_filename.empty())
	{
		// 读取intrinsic
		FileStorage fs(intrinsic_filename, FileStorage::READ);
		if (!fs.isOpened())
		{
			//printf("Failed to open file %s\n", intrinsic_filename.c_str());
			return -1;
		}

		Mat M1, D1, M2, D2;
		fs["M1"] >> M1;
		fs["D1"] >> D1;
		fs["M2"] >> M2;
		fs["D2"] >> D2;

		M1 *= scale;
		M2 *= scale;

		// 读取extrinsic
		fs.open(extrinsic_filename, FileStorage::READ);
		if (!fs.isOpened())
		{
			//printf("Failed to open file %s\n", extrinsic_filename.c_str());
			return -1;
		}

		Mat R, T, R1, P1, R2, P2;
		fs["R"] >> R;
		fs["T"] >> T;

		// 双目矫正参数
		stereoRectify(M1, D1, M2, D2, imgSize, R, T, R1, R2, P1, P2, Q, CALIB_ZERO_DISPARITY, -1, imgSize, &roi1, &roi2);
		
		// 矫正畸变
		Mat map11, map12, map21, map22;
		initUndistortRectifyMap(M1, D1, R1, P1, imgSize, CV_16SC2, map11, map12);
		initUndistortRectifyMap(M2, D2, R2, P2, imgSize, CV_16SC2, map21, map22);

		Mat img1r, img2r;
		remap(left, img1r, map11, map12, INTER_LINEAR);
		remap(right, img2r, map21, map22, INTER_LINEAR);

		left = img1r;
		right = img2r;
	}

	// 算法参数设置
	numberOfDisparities = numberOfDisparities > 0 ? numberOfDisparities : ((imgSize.width / 8) + 15) & -16;

	bm->setROI1(roi1);
	bm->setROI2(roi2);
	bm->setPreFilterCap(31);
	bm->setBlockSize(SADWindowSize > 0 ? SADWindowSize : 9);
	bm->setMinDisparity(0);
	bm->setNumDisparities(numberOfDisparities);
	bm->setTextureThreshold(10);
	bm->setUniquenessRatio(15);
	bm->setSpeckleWindowSize(100);
	bm->setSpeckleRange(32);
	bm->setDisp12MaxDiff(1);

	sgbm->setPreFilterCap(32);	// 预处理滤波器的截断（原63）
	int sgbmWinSize = SADWindowSize > 0 ? SADWindowSize : 3;
	sgbm->setBlockSize(sgbmWinSize);

	int cn = left.channels();

	sgbm->setP1(8 * cn*sgbmWinSize*sgbmWinSize);
	sgbm->setP2(32 * cn*sgbmWinSize*sgbmWinSize);
	sgbm->setMinDisparity(0);
	sgbm->setNumDisparities(numberOfDisparities);
	sgbm->setUniquenessRatio(10);
	sgbm->setSpeckleWindowSize(100);
	sgbm->setSpeckleRange(32);
	sgbm->setDisp12MaxDiff(1);
	if (alg == STEREO_HH)
		sgbm->setMode(StereoSGBM::MODE_HH);
	else if (alg == STEREO_SGBM)
		sgbm->setMode(StereoSGBM::MODE_SGBM);
	else if (alg == STEREO_HH4)
		sgbm->setMode(StereoSGBM::MODE_HH4);
	else if (alg == STEREO_3WAY)
		sgbm->setMode(StereoSGBM::MODE_SGBM_3WAY);

	// 计算
	Mat disp, disp8;

	int64 t = getTickCount();	// 操作系统启动已过去毫秒数，最后为计算时间
	float disparity_multiplier = 1.0f;

	if (alg == STEREO_BM)
	{
		bm->compute(left, right, disp);
		if (disp.type() == CV_16S)
			disparity_multiplier = 16.0f;
	}
	else if (alg == STEREO_SGBM || alg == STEREO_HH || alg == STEREO_HH4 || alg == STEREO_3WAY)
	{
		sgbm->compute(left, right, disp);
		if (disp.type() == CV_16S)
			disparity_multiplier = 16.0f;
	}

	t = getTickCount() - t;
	if(outFlag)
		printf("Time elapsed: %fms\n", t * 1000 / getTickFrequency());

	if (alg != STEREO_VAR)
		disp.convertTo(disp8, CV_8U, 255 / (numberOfDisparities*16.));
	else
		disp.convertTo(disp8, CV_8U);

	// 显示
	Mat disp8_3c;
	// 显示颜色
	if (color_display)
	{	
		cv::applyColorMap(disp8, disp8_3c, COLORMAP_TURBO);
	}

	// 写入深度图
	if (!disparity_filename.empty())
	{
		imwrite(disparity_filename, color_display ? disp8_3c : disp8);
	}
	
	// 写入点云图
	if (!point_cloud_filename.empty())
	{
		printf("storing the point cloud...");
		fflush(stdout);
		Mat xyz;
		Mat floatDisp;
		disp.convertTo(floatDisp, CV_32F, 1.0f / disparity_multiplier);
		reprojectImageTo3D(floatDisp, xyz, Q, true);
		saveXYZ(point_cloud_filename.c_str(), xyz);
		printf("\n");
	}

	//// 显示
	//if (!no_display)
	//{
	//	std::ostringstream oss;
	//	oss << "disparity  " << (alg == STEREO_BM ? "bm" :
	//		alg == STEREO_SGBM ? "sgbm" :
	//		alg == STEREO_HH ? "hh" :
	//		alg == STEREO_VAR ? "var" :
	//		alg == STEREO_HH4 ? "hh4" :
	//		alg == STEREO_3WAY ? "sgbm3way" : "");
	//	oss << "  blocksize:" << (alg == STEREO_BM ? SADWindowSize : sgbmWinSize);
	//	oss << "  max-disparity:" << numberOfDisparities;
	//	std::string disp_name = oss.str();

	//	namedWindow("left", cv::WINDOW_NORMAL);
	//	imshow("left", left);
	//	namedWindow("right", cv::WINDOW_NORMAL);
	//	imshow("right", right);
	//	namedWindow(disp_name, cv::WINDOW_AUTOSIZE);
	//	imshow(disp_name, color_display ? disp8_3c : disp8);

	//	printf("press ESC key or CTRL+C to close...");
	//	fflush(stdout);
	//	printf("\n");
	//	while (1)
	//	{
	//		if (waitKey() == 27) //ESC (prevents closing on actions like taking screenshots)
	//			break;
	//	}
	//}

	*depth = color_display ? disp8_3c : disp8;
	return 0;
}
