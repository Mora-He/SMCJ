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
	int SADWindowSize = 9, numberOfDisparities = -1;		// SAD���ڴ�С��ƥ����С��������Ӳ�(ԭ256)����������������������
	bool no_display = true, color_display = false;
	float scale = 0.3;
	string intrinsic_filename = speFlag ? "intrinsic.yml" : "";	// ����
	string extrinsic_filename = speFlag ? "extrinsic.yml" : "";
	string disparity_filename = fileFlag ? "disparity.jpg" : "";	// �Ӳ�ͼ�ļ�
	string point_cloud_filename = fileFlag ? "point_cloud.pcd" : "";	// ����ͼ�ļ�

	Ptr<StereoBM> bm = StereoBM::create(16, 9);
	Ptr<StereoSGBM> sgbm = StereoSGBM::create(0, 16, 3);

	// �п�
	if (leftFrame.empty() || rightFrame.empty())
	{
		cout << "error:img empty" << endl;
		return -1;
	}

	// ͼƬ����
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

	// �������
	if (!intrinsic_filename.empty() && !extrinsic_filename.empty())
	{
		// ��ȡintrinsic
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

		// ��ȡextrinsic
		fs.open(extrinsic_filename, FileStorage::READ);
		if (!fs.isOpened())
		{
			//printf("Failed to open file %s\n", extrinsic_filename.c_str());
			return -1;
		}

		Mat R, T, R1, P1, R2, P2;
		fs["R"] >> R;
		fs["T"] >> T;

		// ˫Ŀ��������
		stereoRectify(M1, D1, M2, D2, imgSize, R, T, R1, R2, P1, P2, Q, CALIB_ZERO_DISPARITY, -1, imgSize, &roi1, &roi2);
		
		// ��������
		Mat map11, map12, map21, map22;
		initUndistortRectifyMap(M1, D1, R1, P1, imgSize, CV_16SC2, map11, map12);
		initUndistortRectifyMap(M2, D2, R2, P2, imgSize, CV_16SC2, map21, map22);

		Mat img1r, img2r;
		remap(left, img1r, map11, map12, INTER_LINEAR);
		remap(right, img2r, map21, map22, INTER_LINEAR);

		left = img1r;
		right = img2r;
	}

	// �㷨��������
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

	sgbm->setPreFilterCap(32);	// Ԥ�����˲����Ľضϣ�ԭ63��
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

	// ����
	Mat disp, disp8;

	int64 t = getTickCount();	// ����ϵͳ�����ѹ�ȥ�����������Ϊ����ʱ��
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

	// ��ʾ
	Mat disp8_3c;
	// ��ʾ��ɫ
	if (color_display)
	{	
		cv::applyColorMap(disp8, disp8_3c, COLORMAP_TURBO);
	}

	// д�����ͼ
	if (!disparity_filename.empty())
	{
		imwrite(disparity_filename, color_display ? disp8_3c : disp8);
	}
	
	// д�����ͼ
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

	//// ��ʾ
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
