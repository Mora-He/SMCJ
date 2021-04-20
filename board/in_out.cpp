#include "in_out.h"

int main()
{
	struct ImageData left;
	struct ImageData right;
	struct ImageData result;

	cv::Mat left_mat = cv::imread("right.jpg");
	cv::Mat right_mat = cv::imread("right.jpg");

	cv::Size size(std::max(left_mat.cols, right_mat.cols), std::max(left_mat.rows, right_mat.rows));
	cv::Mat result_mat;
	
	result_mat.create(size, CV_MAKETYPE(left_mat.depth(), 3));

	result.data = result_mat.data;

	left.data = left_mat.data;
	left.channels = left_mat.channels();
	left.height = left_mat.rows;
	left.width = left_mat.cols;

	right.data = right_mat.data;
	right.channels = right_mat.channels();
	right.height = right_mat.rows;
	right.width = right_mat.cols;
	// ****************** 测距部分 *********************

	// ************************************************
	detect(&left, &right, &result);

	cv::imshow("merge", result_mat);
	cv::waitKey();
	return 0;
}

// 左右的合成
void detect(struct ImageData* left, struct ImageData* right, struct ImageData* result)
{
	cv::Mat left_mat(left->height, left->width, CV_8UC3);
	cv::Mat right_mat(right->height, right->width, CV_8UC3);
	cv::Mat left_small, right_small;
	cv::Mat outLeft, outRight;
	
	cv::Size size(std::max(left_mat.cols, right_mat.cols), std::max(left_mat.rows, right_mat.rows));
	cv::Mat result_mat;

	left_mat.data = left->data;
	right_mat.data = right->data;

	result_mat.create(size, CV_MAKETYPE(left_mat.depth(), 3));
	result_mat.data = result->data;
	result_mat = cvScalarAll(0);

	cv::resize(left_mat, left_small, cv::Size(), 0.5, 0.5, cv::INTER_AREA);
	cv::resize(right_mat, right_small, cv::Size(), 0.5, 0.5, cv::INTER_AREA);


	result->channels = result_mat.channels();
	result->width = result_mat.cols;
	result->height = result_mat.rows;

	outLeft = result_mat(cv::Rect(0, 0, left_small.cols, left_small.rows));
	outRight = result_mat(cv::Rect(result_mat.cols / 2, 0, right_small.cols, right_small.rows));

	left_small.copyTo(outLeft);
	right_small.copyTo(outRight);
	return;
}
