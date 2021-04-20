#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "seeta_face2_detect_adpt.h"
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
int SendPic(const VIDEO_FRAME_INFO_S *left, const VIDEO_FRAME_INFO_S *right, struct ImageData *result_id);

void detect(struct ImageData* left, struct ImageData* right, struct ImageData* result);