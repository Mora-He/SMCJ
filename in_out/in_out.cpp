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
	
	//result_mat.create(size, CV_MAKETYPE(left_mat.depth(), 3));
	cv::cvtColor(right_mat, result_mat, CV_RGB2YUV_I420);
	result.data = result_mat.data;
	
	left.data = left_mat.data;
	left.channels = left_mat.channels();
	left.height = left_mat.rows;
	left.width = left_mat.cols;

	right.data = right_mat.data;
	right.channels = right_mat.channels();
	right.height = right_mat.rows;
	right.width = right_mat.cols;

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
	cv::Mat result_rgb;

	left_mat.data = left->data;
	right_mat.data = right->data;

	result_rgb.create(size, CV_MAKETYPE(left_mat.depth(), 3));
	//result_mat.data = result->data;
	result_rgb = cvScalarAll(0);

	cv::resize(left_mat, left_small, cv::Size(), 0.5, 0.5, cv::INTER_AREA);
	cv::resize(right_mat, right_small, cv::Size(), 0.5, 0.5, cv::INTER_AREA);


	result->channels = result_rgb.channels();
	result->width = result_rgb.cols;
	result->height = result_rgb.rows;

	outLeft = result_rgb(cv::Rect(0, 0, left_small.cols, left_small.rows));
	outRight = result_rgb(cv::Rect(result_rgb.cols / 2, 0, right_small.cols, right_small.rows));

	left_small.copyTo(outLeft);
	right_small.copyTo(outRight);

	cv::Mat result_yuv;
	cv::cvtColor(result_rgb, result_yuv, CV_RGB2YUV_I420);

	cv::cvtColor(result_rgb, result_yuv, CV_RGB2YUV_I420);
	memcpy(result->data, result_yuv.data, sizeof(uchar)*result_yuv.size().height*result_yuv.size().width);
	return;
}
