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




int SendPic(StereoMatch *matcher, const VIDEO_FRAME_INFO_S *left, const VIDEO_FRAME_INFO_S *right, HI_U32 X, HI_U32 Y)
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
	uchar depth;
	matcher->stereo_match(left_mat,right_mat,X,Y,&depth);
	std::cout << left_mat.size() << std::endl;
	// the function who gives the result
	// iRet = Composite(&left_mat,&right_mat,&result_mat);
	// std::string path;
	// char path_left[8] = "left";
	// char path_right[8] = "right";

	// if(count % 400 == 0)
	// {
	// 	printf("count:%d\n",count);
	// 	// strcpy(path,path_left);
	// 	path = "/mnt/app/photos/left" + std::to_string(count/400) + ".jpg";
	// 	// strcat(path,std::to_string(count/1000));
	// 	imwrite(path,left_mat);
	// 	// strcpy(path,path_right);
	// 	path = "/mnt/app/photos/right" + std::to_string(count/400) + ".jpg";
	// 	// strcat(path,itoa(count/1000));
	// 	imwrite(path,right_mat);
	// }

	iRet = HI_SUCCESS;

EXIT_WITH_FREE:
	HI_MPI_SYS_MmzFree(u64LeftBgr888PicBufPhyAddr, pcLeftBgr888PicBufVirAddr);
	HI_MPI_SYS_MmzFree(u64RightBgr888PicBufPhyAddr, pcRightBgr888PicBufVirAddr);

EXIT:
	return iRet;
}


// 左右的合成
// int Composite(struct ImageData* left, struct ImageData* right, struct ImageData* result)

