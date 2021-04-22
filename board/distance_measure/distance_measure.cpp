#include "distance_measure.hpp"
// #include "seeta_face2_detect_adpt.h"
static int ConvYuvFrmToBgr888(const VIDEO_FRAME_INFO_S *pstYuv420spPic, 
		HI_U64 u64Bgr888PicBufPhyAddr, char *pcBgr888PicBufVirAddr)
{
    int iRet = HI_FAILURE;
	IVE_HANDLE hIve;
	IVE_SRC_IMAGE_S stIveSrcImg;
	IVE_DST_IMAGE_S stIveDstImg;
	IVE_CSC_CTRL_S stIveCscCtrl;
	HI_BOOL bFinish = HI_FALSE;
	
	memset(&stIveSrcImg, 0, sizeof(stIveSrcImg));
	stIveSrcImg.enType = IVE_IMAGE_TYPE_YUV420SP;
	stIveSrcImg.au64PhyAddr[0] = pstYuv420spPic->stVFrame.u64PhyAddr[0];
	stIveSrcImg.au64PhyAddr[1] = pstYuv420spPic->stVFrame.u64PhyAddr[1];
	stIveSrcImg.au64PhyAddr[2] = pstYuv420spPic->stVFrame.u64PhyAddr[2];
	stIveSrcImg.au64VirAddr[0] = (HI_U64)(pstYuv420spPic->stVFrame.u64VirAddr[0]);
	stIveSrcImg.au64VirAddr[1] = (HI_U64)(pstYuv420spPic->stVFrame.u64VirAddr[1]);
	stIveSrcImg.au64VirAddr[2] = (HI_U64)(pstYuv420spPic->stVFrame.u64VirAddr[2]);
	stIveSrcImg.au32Stride[0] = pstYuv420spPic->stVFrame.u32Stride[0];
	stIveSrcImg.au32Stride[1] = pstYuv420spPic->stVFrame.u32Stride[1];
	stIveSrcImg.au32Stride[2] = pstYuv420spPic->stVFrame.u32Stride[2];
	stIveSrcImg.u32Width = pstYuv420spPic->stVFrame.u32Width;
	stIveSrcImg.u32Height = pstYuv420spPic->stVFrame.u32Height;

	memset(&stIveDstImg, 0, sizeof(stIveDstImg));
	stIveDstImg.enType = IVE_IMAGE_TYPE_U8C3_PACKAGE;
	stIveDstImg.au64PhyAddr[0] = u64Bgr888PicBufPhyAddr;
	stIveDstImg.au64VirAddr[0] = (HI_U64)pcBgr888PicBufVirAddr;
	stIveDstImg.au32Stride[0] = stIveSrcImg.u32Width;
	stIveDstImg.u32Width = stIveSrcImg.u32Width;
	stIveDstImg.u32Height = stIveSrcImg.u32Height;

	memset(&stIveCscCtrl, 0, sizeof(stIveCscCtrl));
	stIveCscCtrl.enMode = IVE_CSC_MODE_PIC_BT709_YUV2RGB;
	iRet = HI_MPI_IVE_CSC(&hIve, &stIveSrcImg, &stIveDstImg, &stIveCscCtrl, HI_TRUE);
	if (HI_SUCCESS != iRet)
	{
		HI_ERR_PRINT("HI_MPI_IVE_CSC() failed.\n");
		goto EXIT;
	}

	iRet = HI_MPI_IVE_Query(hIve, &bFinish, HI_TRUE);
	if ((HI_SUCCESS == iRet) && (!bFinish))
	{
		iRet = HI_FAILURE;
	}

EXIT:
	return iRet;
}




int SendPic(const VIDEO_FRAME_INFO_S *left, const VIDEO_FRAME_INFO_S *right, struct ImageData *result_id, long count)
// int SendPic(const cv::Mat *left, const cv::Mat *right, cv::Mat *result_id)
{
	int iRet = HI_FAILURE;
	// structs of input
	struct ImageData left_id;
	struct ImageData right_id;
	// struct ImageData result_id;
	struct ImageData result;
	
	HI_U64 u64LeftBgr888PicBufPhyAddr;
	HI_U64 u64RightBgr888PicBufPhyAddr;
	char *pcLeftBgr888PicBufVirAddr;
	char *pcRightBgr888PicBufVirAddr;
	HI_U32 u32LeftBufLen;
	HI_U32 u32RightBufLen;

	u32LeftBufLen = left->stVFrame.u32Width * left->stVFrame.u32Height * 3;
	iRet = HI_MPI_SYS_MmzAlloc_Cached(&u64LeftBgr888PicBufPhyAddr, 
			(HI_VOID**)&pcLeftBgr888PicBufVirAddr, NULL, NULL, u32LeftBufLen);
	if (HI_SUCCESS != iRet)
	{	
		printf("HI_MPI_SYS_MmzAlloc_Cached() failed.\n");
		goto EXIT;
	}
	u32RightBufLen = right->stVFrame.u32Width * right->stVFrame.u32Height * 3;
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

	cv::Mat left_mat(left_id.height, left_id.width, CV_8UC3, pcLeftBgr888PicBufVirAddr);
	cv::Mat right_mat(right_id.height, right_id.width, CV_8UC3, pcRightBgr888PicBufVirAddr);
	cv::Mat result_mat(right_id.height, right_id.width, CV_8UC3);
	// the function who gives the result
	// iRet = Composite(&left_mat,&right_mat,&result_mat);
	std::string path;
	char path_left[8] = "left";
	char path_right[8] = "right";

	if(count % 400 == 0)
	{
		printf("count:%d\n",count);
		// strcpy(path,path_left);
		path = "/mnt/app/photos/left" + std::to_string(count/400) + ".jpg";
		// strcat(path,std::to_string(count/1000));
		imwrite(path,left_mat);
		// strcpy(path,path_right);
		path = "/mnt/app/photos/right" + std::to_string(count/400) + ".jpg";
		// strcat(path,itoa(count/1000));
		imwrite(path,right_mat);
	}

	iRet = HI_SUCCESS;

EXIT_WITH_FREE:
	HI_MPI_SYS_MmzFree(u64LeftBgr888PicBufPhyAddr, pcLeftBgr888PicBufVirAddr);
	HI_MPI_SYS_MmzFree(u64RightBgr888PicBufPhyAddr, pcRightBgr888PicBufVirAddr);

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

// 	Composite(&left, &right, &result);

// 	cv::imshow("merge", result_mat);
// 	cv::waitKey();
// 	return 0;
// }

// 左右的合成
// int Composite(struct ImageData* left, struct ImageData* right, struct ImageData* result)
int Composite(cv::Mat *left, cv::Mat *right, cv::Mat *result)
{
	// cv::Mat left_mat(left->height, left->width, CV_8UC3);
	// cv::Mat right_mat(right->height, right->width, CV_8UC3);
	// cv::Mat left_small, right_small;
	// cv::Mat outLeft, outRight;
	
	// cv::Size size(std::max(left_mat.cols, right_mat.cols), std::max(left_mat.rows, right_mat.rows));
	// cv::Mat result_mat;

	// left_mat.data = left->data;
	// right_mat.data = right->data;

	// result_mat.create(size, CV_MAKETYPE(left_mat.depth(), 3));
	// result_mat.data = result->data;
	// result_mat = cvScalarAll(0);

	// cv::resize(left_mat, left_small, cv::Size(), 0.5, 0.5, cv::INTER_AREA);
	// cv::resize(right_mat, right_small, cv::Size(), 0.5, 0.5, cv::INTER_AREA);


	// result->channels = result_mat.channels();
	// result->width = result_mat.cols;
	// result->height = result_mat.rows;

	// outLeft = result_mat(cv::Rect(0, 0, left_small.cols, left_small.rows));
	// outRight = result_mat(cv::Rect(result_mat.cols / 2, 0, right_small.cols, right_small.rows));

	// left_small.copyTo(outLeft);
	// right_small.copyTo(outRight);
	// printf("left width:%d,left height:%d;right width:%d,right height:%d",
	// 		left->cols,left->rows,right->cols,right->rows);
	return HI_SUCCESS;
}
