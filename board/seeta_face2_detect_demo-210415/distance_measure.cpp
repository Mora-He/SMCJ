#include "distance_measure.h"
#include "seeta_face2_detect_adpt.h"
int SendPic(const VIDEO_FRAME_INFO_S *left, const VIDEO_FRAME_INFO_S *right, struct ImageData *result_id)
{
	int iRet = HI_FAILURE;
	// structs of input
	struct ImageData left_id;
	struct ImageData right_id;
	// struct ImageData result_id;

	HI_U64 u64LeftBgr888PicBufPhyAddr;
	HI_U64 u64RightBgr888PicBufPhyAddr;
	char *pcLeftBgr888PicBufVirAddr;
	char *pcRightBgr888PicBufVirAddr;
	HI_U32 u32LeftBufLen;
	HI_U32 u32RightBufLen;

	u32LeftBufLen = left->stVFrame.u32Width * pstCatchYuvFrm->stVFrame.u32Height * 3;
	iRet = HI_MPI_SYS_MmzAlloc_Cached(&u64LeftBgr888PicBufPhyAddr, 
			(HI_VOID**)&pcLeftBgr888PicBufVirAddr, NULL, NULL, u32LeftBufLen);
	if (HI_SUCCESS != iRet)
	{	
		printf("HI_MPI_SYS_MmzAlloc_Cached() failed.\n");
		goto EXIT;
	}
	u32RightBufLen = right->stVFrame.u32Width * pstCatchYuvFrm->stVFrame.u32Height * 3;
	iRet = HI_MPI_SYS_MmzAlloc_Cached(&u64RightBgr888PicBufPhyAddr, 
			(HI_VOID**)&pcRightBgr888PicBufVirAddr, NULL, NULL, u32RightBufLen);
	if (HI_SUCCESS != iRet)
	{	
		printf("HI_MPI_SYS_MmzAlloc_Cached() failed.\n");
		goto EXIT;
	}
	iRet = ConvYuvFrmToBgr888(left, u64LeftBgr888PicBufPhyAddr, pcLeftBgr888PicBufVirAddr);
	if (HI_SUCCESS != iRet)
	{
		printf("ConvYuvFrmToBgr888() failed.\n");
		goto EXIT_WITH_FREE;
	}
	iRet = ConvYuvFrmToBgr888(right, u64RightBgr888PicBufPhyAddr, pcRightBgr888PicBufVirAddr);
	if (HI_SUCCESS != iRet)
	{
		printf("ConvYuvFrmToBgr888() failed.\n");
		goto EXIT_WITH_FREE;
	}
	
	HI_MPI_SYS_MmzFlushCache(u64LeftBgr888PicBufPhyAddr, pcLeftBgr888PicBufVirAddr, u32LeftBufLen);
	HI_MPI_SYS_MmzFlushCache(u64RightBgr888PicBufPhyAddr, pcRightBgr888PicBufVirAddr, u32RightBufLen);

	left_id.data = (unsigned char *)pcLeftBgr888PicBufVirAddr;
	left_id.width = left -> stVFrame.u32Width;
	left_id.height = left -> stVFrame.u32Height;
	left_id.channels = 3;
	right_id.data = (unsigned char *)pcLeftBgr888PicBufVirAddr;
	right_id.width = right -> stVFrame.u32Width;
	right_id.height = right -> stVFrame.u32Height;
	right_id.channels = 3;

	// the function who gives the result
	iRet = detect(left_id,right_id,result_id)

	iRet = HI_SUCCESS;

EXIT_WITH_FREE:
	HI_MPI_SYS_MmzFree(u64Bgr888PicBufPhyAddr, pcBgr888PicBufVirAddr);

EXIT:
	return iRet;
}
// int main()
// {
// 	struct ImageData left;
// 	struct ImageData right;
// 	struct ImageData result;

// 	cv::Mat left_mat = cv::imread("right.jpg");
// 	cv::Mat right_mat = cv::imread("right.jpg");

// 	cv::Size size(std::max(left_mat.cols, right_mat.cols), std::max(left_mat.rows, right_mat.rows));
// 	cv::Mat result_mat;
	
// 	result_mat.create(size, CV_MAKETYPE(left_mat.depth(), 3));

// 	result.data = result_mat.data;

// 	left.data = left_mat.data;
// 	left.channels = left_mat.channels();
// 	left.height = left_mat.rows;
// 	left.width = left_mat.cols;

// 	right.data = right_mat.data;
// 	right.channels = right_mat.channels();
// 	right.height = right_mat.rows;
// 	right.width = right_mat.cols;

// 	detect(&left, &right, &result);

// 	cv::imshow("merge", result_mat);
// 	cv::waitKey();
// 	return 0;
// }

// 左右的合成
int detect(struct ImageData* left, struct ImageData* right, struct ImageData* result)
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
