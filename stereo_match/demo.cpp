#include "demo.h"

int main()
{
	cv::Mat left, right;
	int x = 320, y = 240;
	uchar depth;
	int64 t;
	StereoMatch matcher;

	matcher.init();		// 初始化

	while (true)
	{
		left = cv::imread("data/left1.jpg", -1);
		right = cv::imread("data/right1.jpg", -1);

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
		int result = depth;
		std::cout << result << std::endl;
		t = cv::getTickCount() - t;
		printf("Time elapsed:%fms\n", t * 1000 / cv::getTickFrequency());
	}

	return EXIT_SUCCESS;
}
