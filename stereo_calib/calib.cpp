#include "calib.h"

void stereo_calib(std::vector<cv::Mat> imagelist, cv::Size boardSize, float squareSize)
{
	bool useCalibrated = true;
	bool showRectified = true;
	std::string intrinsic_filename = "intrinsic.yml";

	if (imagelist.size() % 2 != 0)
	{
		std::cout << "error: the number of imagelist is error" << std::endl;
		return;
	}

	std::vector<std::vector<cv::Point2f>> imagePoints[2];
	std::vector<std::vector<cv::Point3f>> objectPoints;
	cv::Size imageSize = imagelist[0].size();
	std::vector<cv::Mat> goodImagelist;
	const int maxScale = 2;

	int i, j, k, nimages = (int)imagelist.size() / 2;
	imagePoints[0].resize(nimages);
	imagePoints[1].resize(nimages);

	// 角点检测，一左一右对应
	for (i = j = 0; i < nimages; i++)
	{
		std::cout << "i" << i + 1 << std::endl;
		for (k = 0; k < 2; k++)
		{
			cv::Mat img = imagelist[i * 2 + k];

			if (img.empty()) break;
			// 图像大小一致
			if (imageSize != img.size())
			{
				std::cout << "The images have different size" << std::endl;
				break;
			}

			bool found = false;
			std::vector<cv::Point2f>&corners = imagePoints[k][j];
			// 同一图像压缩进行多次检测，取可检测的一次
			for (int scale = 1; scale <= maxScale; scale++)
			{
				cv::Mat timg;
				if (scale == 1)
				{
					timg = img;
				}
				else
				{
					resize(img, timg, cv::Size(), scale, scale, cv::INTER_LINEAR_EXACT);
				}

				found = cv::findChessboardCorners(timg, boardSize, corners, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE);
				std::cout << "found" << found << std::endl;

				if (found)
				{
					if (scale > 1)
					{
						cv::Mat cornersMat(corners);
						cornersMat *= 1. / scale;
					}
					break;
				}
			}

			if (!found)
				break;

			cv::Mat gray;
			//cvtColor(img, gray, cv::COLOR_RGB2GRAY);
			gray = img;	//test/////////////////////////////////////////////////////
			cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1),
				cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS,
					30, 0.01));		// 亚像素角点检测
		}

		if (k == 2)
		{
			goodImagelist.push_back(imagelist[i * 2]);
			goodImagelist.push_back(imagelist[i * 2 + 1]);
			j++;
		}
	}

	// 最终成对
	nimages = j;
	if (nimages < 2)
	{
		std::cout << "error:too little pairs to run the calibration\n";
		return;
	}

	// 标定初始化
	imagePoints[0].resize(nimages);
	imagePoints[1].resize(nimages);
	objectPoints.resize(nimages);

	for (i = 0; i < nimages; i++)
	{
		for (j = 0; j < boardSize.height; j++)
		{
			for (k = 0; k < boardSize.width; k++)
				objectPoints[i].push_back(cv::Point3f(k*squareSize, j*squareSize, 0));
		}
	}

	cv::Mat cameraMatrix[2], distCoeffs[2];
	cv::Mat R, T, E, F;
	cameraMatrix[0] = initCameraMatrix2D(objectPoints, imagePoints[0], imageSize, 0);
	cameraMatrix[1] = initCameraMatrix2D(objectPoints, imagePoints[1], imageSize, 0);

	// 标定
	double rms = stereoCalibrate(objectPoints, imagePoints[0], imagePoints[1],
		cameraMatrix[0], distCoeffs[0],
		cameraMatrix[1], distCoeffs[1],
		imageSize, R, T, E, F,
		cv::CALIB_FIX_ASPECT_RATIO +
		cv::CALIB_SAME_FOCAL_LENGTH +
		cv::CALIB_RATIONAL_MODEL + 
		cv::CALIB_FIX_PRINCIPAL_POINT +
		//cv::CALIB_FIX_K1 + 
		//cv::CALIB_FIX_K2 + 
		cv::CALIB_FIX_K3 + 
		cv::CALIB_FIX_K4 + 
		cv::CALIB_FIX_K5 + 
		cv::CALIB_FIX_K6 +
		cv::CALIB_FIX_S1_S2_S3_S4,
		cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 100, 1e-5));
	std::cout << "done with RMS error=" << rms << std::endl;


	// 误差 m2^t*F*m1=0
	double err = 0;
	int npoints = 0;
	std::vector<cv::Vec3f> lines[2];
	for (i = 0; i < nimages; i++)
	{
		int npt = (int)imagePoints[0][i].size();
		cv::Mat imgpt[2];
		for (k = 0; k < 2; k++)
		{
			imgpt[k] = cv::Mat(imagePoints[k][i]);
			undistortPoints(imgpt[k], imgpt[k], cameraMatrix[k], distCoeffs[k], cv::Mat(), cameraMatrix[k]);
			computeCorrespondEpilines(imgpt[k], k + 1, F, lines[k]);
		}
		for (j = 0; j < npt; j++)
		{
			double errij = fabs(imagePoints[0][i][j].x*lines[1][j][0] +
				imagePoints[0][i][j].y*lines[1][j][1] + lines[1][j][2]) +
				fabs(imagePoints[1][i][j].x*lines[0][j][0] +
					imagePoints[1][i][j].y*lines[0][j][1] + lines[0][j][2]);
			err += errij;
		}
		npoints += npt;
	}
	std::cout << "average epipolar err = " << err / npoints << std::endl;

	cv::Mat R1, R2, P1, P2, Q;
	cv::Rect validRoi[2];
	cv::stereoRectify(cameraMatrix[0], distCoeffs[0],
		cameraMatrix[1], distCoeffs[1],
		imageSize, R, T, R1, R2, P1, P2, Q,
		cv::CALIB_ZERO_DISPARITY, 1, imageSize, &validRoi[0], &validRoi[1]);

	// 标定结果保存
	cv::FileStorage fs(intrinsic_filename, cv::FileStorage::WRITE);
	if (fs.isOpened())
	{
		fs << "M1" << cameraMatrix[0] << "D1" << distCoeffs[0] <<
			"M2" << cameraMatrix[1] << "D2" << distCoeffs[1] << 
			"R" << R << "T" << T << "R1" << R1 << "R2" << R2 << "P1" << P1 << "P2" << P2 << "Q" << Q <<
			"roi1" << validRoi[0] << "roi2" << validRoi[1];
		fs.release();
	}
	else
		std::cout << "Error: can not save the extrinsic parameters\n";

	// OpenCV can handle left-right
	// or up-down camera arrangements 显示矫正图片
	bool isVerticalStereo = fabs(P2.at<double>(1, 3)) > fabs(P2.at<double>(0, 3));

	if (!showRectified)
		return;

	

	// BOUGUET'S 方法
	if (useCalibrated)
	{
		// we already computed everything
	}
	//// HARTLEY'S方法
	//else
	//	// use intrinsic parameters of each camera, but
	//	// compute the rectification transformation directly
	//	// from the fundamental matrix
	//{
	//	std::vector<cv::Point2f> allimgpt[2];
	//	for (k = 0; k < 2; k++)
	//	{
	//		for (i = 0; i < nimages; i++)
	//			std::copy(imagePoints[k][i].begin(), imagePoints[k][i].end(), back_inserter(allimgpt[k]));
	//	}
	//	F = cv::findFundamentalMat(cv::Mat(allimgpt[0]), cv::Mat(allimgpt[1]), cv::FM_8POINT, 0, 0);
	//	cv::Mat H1, H2;
	//	stereoRectifyUncalibrated(cv::Mat(allimgpt[0]), cv::Mat(allimgpt[1]), F, imageSize, H1, H2, 3);

	//	R1 = cameraMatrix[0].inv()*H1*cameraMatrix[0];
	//	R2 = cameraMatrix[1].inv()*H2*cameraMatrix[1];
	//	P1 = cameraMatrix[0];
	//	P2 = cameraMatrix[1];
	//}

	cv::Mat rmap[2][2];
	//Precompute maps for cv::remap()，坐标映射
	cv::initUndistortRectifyMap(cameraMatrix[0], distCoeffs[0], R1, P1, imageSize, CV_16SC2, rmap[0][0], rmap[0][1]);
	cv::initUndistortRectifyMap(cameraMatrix[1], distCoeffs[1], R2, P2, imageSize, CV_16SC2, rmap[1][0], rmap[1][1]);

	cv::Mat canvas;
	double sf;
	int w, h;
	if (!isVerticalStereo)
	{
		sf = 600. / MAX(imageSize.width, imageSize.height);
		w = cvRound(imageSize.width*sf);
		h = cvRound(imageSize.height*sf);
		canvas.create(h, w * 2, CV_8UC3);
	}
	else
	{
		sf = 300. / MAX(imageSize.width, imageSize.height);
		w = cvRound(imageSize.width*sf);
		h = cvRound(imageSize.height*sf);
		canvas.create(h * 2, w, CV_8UC3);
	}

	for (i = 0; i < nimages; i++)
	{
		for (k = 0; k < 2; k++)
		{
			cv::Mat img = goodImagelist[i * 2 + k], rimg, cimg, gray;
			//cvtColor(img, gray, cv::COLOR_RGB2GRAY);
			gray = img;	// test
			remap(gray, rimg, rmap[k][0], rmap[k][1], cv::INTER_LINEAR);		// 坐标映射
			cv::cvtColor(rimg, cimg, cv::COLOR_GRAY2BGR);
			cv::Mat canvasPart = !isVerticalStereo ? canvas(cv::Rect(w*k, 0, w, h)) : canvas(cv::Rect(0, h*k, w, h));
			resize(cimg, canvasPart, canvasPart.size(), 0, 0, cv::INTER_AREA);
			if (useCalibrated)
			{
				cv::Rect vroi(cvRound(validRoi[k].x*sf), cvRound(validRoi[k].y*sf),
					cvRound(validRoi[k].width*sf), cvRound(validRoi[k].height*sf));
				rectangle(canvasPart, vroi, cv::Scalar(0, 0, 255), 3, 8);
			}
		}

		if (!isVerticalStereo)
			for (j = 0; j < canvas.rows; j += 16)
				line(canvas, cv::Point(0, j), cv::Point(canvas.cols, j), cv::Scalar(0, 255, 0), 1, 8);
		else
			for (j = 0; j < canvas.cols; j += 16)
				line(canvas, cv::Point(j, 0), cv::Point(j, canvas.rows), cv::Scalar(0, 255, 0), 1, 8);
		cv::imshow("rectified", canvas);
		char c = (char)cv::waitKey();
		if (c == 27 || c == 'q' || c == 'Q')
			break;
	}
	cv::destroyWindow("rectified");
	return;
}
