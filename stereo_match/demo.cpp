#include "demo.h"

int main()
{
	cv::Mat left, right;
	int x = 250, y = 480;
	uchar depth;
	int t;
	StereoMatch matcher;

	matcher.init();		// 初始化

	while (true)
	{
		left = cv::imread("data/left1.jpg", -1);
		right = cv::imread("data/right1.jpg", -1);

		std::cout << left.size() << std::endl;
		if (left.empty() || right.empty())
		{
			std::cout << "empty img" << std::endl;
			break;
		}

		t = cv::getTickCount();		// 计时器
		if (matcher.stereo_match(left, right, x, y, &depth) == -1)	// 匹配
		{
			std::cout << "error" << std::endl;
			break;
		}
		t = cv::getTickCount() - t;
		printf("Time elapsed:%fms\n", t * 1000 / cv::getTickFrequency());
	}

	return EXIT_SUCCESS;
}
