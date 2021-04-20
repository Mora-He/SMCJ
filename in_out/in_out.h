#pragma once

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


struct ImageData
{
	unsigned char* data;
	int width;
	int height;
	int channels;
};

void detect(struct ImageData* left, struct ImageData* right, struct ImageData* result);