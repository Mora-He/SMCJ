#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
// #include "seeta_face2_detect_adpt.h"
#include <string.h>
#include "chip_sdk_headers.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <iostream>
#include <vector>
#include "match.hpp"
struct ImageData
{
	unsigned char* data;
	int width;
	int height;
	int channels;
};

// int ConvFrameToMat(const VIDEO_FRAME_INFO_S frame, cv::Mat *result);
// int SendPic(const VIDEO_FRAME_INFO_S *left, const VIDEO_FRAME_INFO_S *right, struct ImageData *result_id,long count);
// int SendPic(StereoMatch *matcher, const VIDEO_FRAME_INFO_S *left, const VIDEO_FRAME_INFO_S *right, HI_U32 X, HI_U32 Y);
int SendPic(StereoMatch *matcher, const VIDEO_FRAME_INFO_S *left, const VIDEO_FRAME_INFO_S *right);
// int SendPic(StereoMatch *matcher, const VIDEO_FRAME_INFO_S *left, const VIDEO_FRAME_INFO_S *right, HI_U32 X, HI_U32 Y, RECT_S *rectangle);


// int SendPic(const cv::Mat *left, const cv::Mat *right, cv::Mat *result_id);

static int ConvYuvFrmToBgr888(const VIDEO_FRAME_INFO_S *pstYuv420spPic, 
		HI_U64 u64Bgr888PicBufPhyAddr, char *pcBgr888PicBufVirAddr);
// int Composite(struct ImageData *left, struct ImageData *right, struct ImageData *result);
// int Composite(cv::Mat *left, cv::Mat *right, cv::Mat *result);
int GPIO_OUT(unsigned int gpio_chip_num, unsigned int gpio_offset_num, unsigned int gpio_out_val);