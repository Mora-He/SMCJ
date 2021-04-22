#include "demo.h"

int main()
{
	cv::Size boardSize;
	float squareSize;
	std::vector<cv::Mat> imageList;
	boardSize.width = 9;
	boardSize.height = 9;
	squareSize = 2.0;

	for (int i = 1; i < 6; i++)
	{
		cv::Mat left = cv::imread("data\\left" + std::to_string(i)+".jpg", 0);
		cv::Mat right = cv::imread("data\\right" + std::to_string(i) + ".jpg", 0);

		if (left.empty() || right.empty()) std::cout << "????" << std::endl;
		imageList.push_back(left);
		imageList.push_back(right);
	}
	stereo_calib(imageList, boardSize, squareSize);
	return EXIT_SUCCESS;
}
