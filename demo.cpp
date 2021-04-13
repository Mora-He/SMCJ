// demo_0.1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "demo.h"

int main()
{
	//int cameraLeft = 0, cameraRight = 1;
	string cameraLeft = "data\\left1.mp4", cameraRight = "data\\right1.mp4";
	VideoCapture captureLeft(cameraLeft);
	VideoCapture captureRight(cameraRight);

	if (!captureLeft.isOpened() || !captureRight.isOpened())
	{
		cerr << "Can not open camera, testing image..." << endl;
	}

	Mat leftFrame, rightFrame;
	vector<Mat> imageList;
	Size boardSize;
	float squareSize;
	int count = 0;
	int flag = NONE;
	while (captureLeft.isOpened() && captureRight.isOpened())
	{
		// 获取图片
		captureLeft.grab();
		captureRight.grab();
		captureLeft.retrieve(leftFrame);
		captureRight.retrieve(rightFrame);

		// 判空
		if (leftFrame.empty() || rightFrame.empty()) break;

		// 显示
		imshow("left", leftFrame);
		imshow("right", rightFrame);
		int key = waitKey(20);

		// esc退出
		if (key == 27) break;

		// c选择标定
		if (char(key) == 'c')
		{
			flag = RECTIFY;
			count = 0;
			imageList.clear();
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
				// 获取标定图片角点数据
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
				//imageList.clear();
				//for (int i = 1; i < 10; i++)
				//{
				//	Mat img = imread("data\\left0" + to_string(i)+".jpg", 0);
				//	if (img.empty()) cout << "????" << endl;
				//	imageList.push_back(img);
				//	Mat img2 = imread("data\\right0" + to_string(i) + ".jpg", 0);
				//	imageList.push_back(img2);
				//}
				cout << imageList.size() << endl;
				stereo_calib(imageList, boardSize, squareSize);
				// 标定成功窗口？
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
	}
	return EXIT_SUCCESS;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
