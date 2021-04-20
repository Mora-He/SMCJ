// demo_0.1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "demo.h"

int main()
{
	//int cameraLeft = 0, cameraRight = 1;
	string cameraLeft = "data\\left1.mp4", cameraRight = "data\\right1.mp4";	// test
	VideoCapture captureLeft(cameraLeft);
	VideoCapture captureRight(cameraRight);

	if (!captureLeft.isOpened() || !captureRight.isOpened())
	{
		cerr << "Can not open camera, testing image..." << endl;
	}

	Mat leftFrame, rightFrame, disp;
	vector<Mat> imageList;
	Size boardSize;
	float squareSize;
	int count = 0;
	int flag = NONE;
	float result = 0;

	// 空图片预处理
	Mat emptyImg;
	string text = "match error";
	empty_img_init(text, &emptyImg);

	int key = 27;
	while (captureLeft.isOpened() && captureRight.isOpened())
	{
		// 获取图片
		captureLeft.grab();
		captureRight.grab();
		captureLeft.retrieve(leftFrame);
		captureRight.retrieve(rightFrame);

		// 判空
		if (leftFrame.empty() || rightFrame.empty()) break;

		// 视差图显示，如果匹配失败则返回空
		// 算法选择输入？？?????????????????????????????
		Mat left = imread("data\\left01.jpg", -1);		// test 灰度图 -1 3秒
		Mat right = imread("data\\right01.jpg", -1);		// test
		//Mat left = imread("data\\left.jpg", -1);		// test 彩色图 -1 5s 3s
		//Mat right = imread("data\\right.jpg", -1);		// test
		//Mat left = imread("data\\aloeL.jpg", -1);		// test 灰度图 -1 3秒
		//Mat right = imread("data\\aloeR.jpg", -1);		// test

		int dispFlag = stereo_match(left, right, &disp, STEREO_SGBM, 1, 1, char(key) == 's');	// test
		//int dispFlag = stereo_match(leftFrame, rightFrame, &disp, STEREO_SGBM, char(key) == 'm', char(key) != 'u', char(key) == 's');	// m匹配时间的输出，u矫正图像，s视差图保存
		//int dispFlag = -1;	// test
		if (dispFlag == -1)
		{
			disp = emptyImg;
		}

		// 显示
		imshow("left", leftFrame);
		imshow("right", rightFrame);/*
		Mat disp_3c;
		cv::applyColorMap(disp, disp_3c, COLORMAP_TURBO);*/
		imshow("disp", disp);
		key = waitKey(20);

		// esc退出
		if (key == 27) break;

		// c选择标定
		if (char(key) == 'c')
		{
			flag = RECTIFY;
			count = 0;
			imageList.clear();
		}		

		// d选择测距
		if (char(key) == 'd')
		{
			flag = DISTANCE;
		}

		// n不选择
		if (char(key) == 'n')
		{
			flag = NONE;
		}

		// 0停止获取标定图片开始标定
		if (flag == RECTIFY && char(key) == '0')
		{
			if (count < 2)
			{
				cout << "error:There is too little image,please push 1 again and get images" << endl;
			}
			else
			{
				// 获取标定图片角点数据??????????????????????????????
				//cout << endl;
				//cout << "boardSize.width:";
				//cin >> boardSize.width;
				//cout << "boardSize.height";
				//cin >> boardSize.height;
				//cout << "squareSize:";
				//cin >> squareSize;
				boardSize.width = 9;
				boardSize.height = 6;
				squareSize = 1.0;

				//////////////////////////////////////////////
				// //calib的test，通道数1
				imageList.clear();
				for (int i = 1; i < 10; i++)
				{
					Mat img = imread("data\\left0" + to_string(i)+".jpg", 0);

					if (img.empty()) cout << "????" << endl;
					imageList.push_back(img);
					Mat img2 = imread("data\\right0" + to_string(i) + ".jpg", 0);
					imageList.push_back(img2);
				}
				for (int i = 0; i < 5; i++)
				{
					Mat img = imread("data\\left1" + to_string(i) + ".jpg", 0);

					if (img.empty()) cout << "????" << endl;
					imageList.push_back(img);
					Mat img2 = imread("data\\right1" + to_string(i) + ".jpg", 0);
					imageList.push_back(img2);
				}
				cout << imageList.size() << endl;
				stereo_calib(imageList, boardSize, squareSize);
				// 标定成功窗口？??????????????????????
			}
			flag = NONE;
		}

		// 1获取标定图片
		if (flag == RECTIFY && char(key) == '1')
		{
			imageList.push_back(leftFrame);
			imageList.push_back(rightFrame);
			count++;
		}
	
		// 测距
		if(flag == DISTANCE)
		//while (flag == DISTANCE)	// test
		{
			Mat depth;	
			disp_to_depth(disp, &depth);	// 测距
			// 位置与距离??????????????????
			//depth.convertTo(depth_3c, CV_8U, 1. / 256);
			//cv::applyColorMap(depth_3c, depth_3c, COLORMAP_TURBO);
			imshow("depth", depth);
		}
	}
	return EXIT_SUCCESS;
}

void empty_img_init(String text, Mat* emptyImg)
{
	Mat img = imread("empty.jpg", -1);
	int fontFace = FONT_HERSHEY_SCRIPT_COMPLEX;		// 字体风格
	double fontScale = 2;	// 缩放系数
	int thickness = 3;	// 笔画线宽
	int baseline = 0;	// 文字最底部y坐标
	int lineType = 12;

	Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
	baseline += thickness;

	Point textOrg((img.cols - textSize.width) / 2, (img.rows + textSize.height) / 2);

	putText(img, text, textOrg, fontFace, fontScale, Scalar::all(0), thickness, lineType);
	
	*emptyImg = img;
	return;
}

void disp_to_depth(Mat disp, Mat* depth)
{
	if (!(disp.type() == CV_8U || disp.type() == CV_8UC3))
		return;

	string intrinsic_filename = "intrinsic.yml";	// 输入
	string extrinsic_filename = "extrinsic.yml";

	// 读取intrinsic
	FileStorage fs(intrinsic_filename, FileStorage::READ);
	if (!fs.isOpened())
	{
		return;
	}

	Mat M1, M2;
	fs["M1"] >> M1;
	fs["M2"] >> M2;

	// 读取extrinsic
	fs.open(extrinsic_filename, FileStorage::READ);
	if (!fs.isOpened())
	{
		return;
	}

	Mat T;
	fs["T"] >> T;

	float fx = (float)(M1.at<double>(0, 0) + M2.at<double>(0, 0)) / 2;	// 取fx为f
	float baseline = (float)T.at<double>(0, 0)*T.at<double>(0, 0)
		+ T.at<double>(1, 0)*T.at<double>(1, 0)
		+ T.at<double>(2, 0)*T.at<double>(2, 0);
	baseline = sqrt(baseline);

	int height = disp.rows;
	int width = disp.cols;

	uchar* dispData = (uchar*)disp.data;
	Mat temp(disp.rows, disp.cols, CV_16UC1);
	ushort* depthData = (ushort*)temp.data;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int id = i * width + j;
			if (!dispData[id]) continue;
			depthData[id] = ushort((float)fx*baseline / (float)dispData[id]);
		}
	}

	*depth = temp;
	cout << depthData[height / 2 * width + width / 2] << endl;


	// 框框
	Mat rec;
	cout << min_element(depthData, depthData+height*width)-depthData << endl;
	threshold(*depth,
		rec,
		(*min_element(depthData, depthData + height * width))+1,
		255,
		CV_THRESH_BINARY);
	imshow("rec", rec);
	waitKey();
	return;
}
