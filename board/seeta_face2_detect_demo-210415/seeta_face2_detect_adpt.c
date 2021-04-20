#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "seeta_face2_detect_adpt.h"

#include "FaceDetector.h"


typedef struct 
{
	seeta::FaceDetector *hpclFaceDetector;
	
}SEETA_FACE2_INSTANCE_CTX_S;


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

int SEETA_FACE2_ADPT_CreateInstance(RT_HANDLE *phInstance)
{
    int iRet = HI_FAILURE;
	SEETA_FACE2_INSTANCE_CTX_S *hpstInstanceCtx;
	//HI_U32 u32ExpectedFacesMaxNum;
	struct SeetaModelSetting stModelSetting;
	const char *apszModel[2];

	hpstInstanceCtx = (SEETA_FACE2_INSTANCE_CTX_S*)malloc(sizeof(SEETA_FACE2_INSTANCE_CTX_S));
	if (NULL == hpstInstanceCtx)
	{
		goto EXIT;
	}

	apszModel[0] = "/mnt/app/resource/model/fd_2_00.dat";
	apszModel[1] = NULL;
	stModelSetting.device = SEETA_DEVICE_CPU;
	stModelSetting.id = 0;
	stModelSetting.model = apszModel;
	hpstInstanceCtx->hpclFaceDetector = new seeta::FaceDetector(stModelSetting);
	if (NULL == hpstInstanceCtx->hpclFaceDetector)
	{
		HI_ERR_PRINT("new failed.\n");
		goto EXIT_WITH_FREE;
	}
	
	hpstInstanceCtx->hpclFaceDetector->set(seeta::FaceDetector::PROPERTY_MIN_FACE_SIZE, 100);
	//hpstInstanceCtx->hpclFaceDetector->set(seeta::FaceDetector::PROPERTY_VIDEO_STABLE, 1);
	//hpstInstanceCtx->hpclFaceDetector->set(seeta::FaceDetector::PROPERTY_THRESHOLD1, 1.0);
	//hpstInstanceCtx->hpclFaceDetector->set(seeta::FaceDetector::PROPERTY_THRESHOLD2, 1.0);
	hpstInstanceCtx->hpclFaceDetector->set(seeta::FaceDetector::PROPERTY_THRESHOLD3, 1.0);

	iRet = HI_SUCCESS;
	*phInstance = (RT_HANDLE)hpstInstanceCtx;
	goto EXIT;

EXIT_WITH_FREE:
	free(hpstInstanceCtx);
EXIT:  
    return iRet;
}

int SEETA_FACE2_ADPT_SendPic(RT_HANDLE hInstance, const VIDEO_FRAME_INFO_S *pstCatchYuvFrm, 
		RECT_S *pstObjRect, HI_U32 *pu32RectNum)
{
	int iRet = HI_FAILURE;
	SEETA_FACE2_INSTANCE_CTX_S *hpstInstanceCtx;
	//vector<FaceRect> stFaceRectVector;
	int iDetectedFaceNum;
	HI_U32 u32Idx;
	struct SeetaImageData simage;
	SeetaFaceInfoArray faces;
	HI_U64 u64Bgr888PicBufPhyAddr;
	char *pcBgr888PicBufVirAddr;
	HI_U32 u32BufLen;
	HI_U32 u32ObjRectNum;

	hpstInstanceCtx = (SEETA_FACE2_INSTANCE_CTX_S*)hInstance;

	u32ObjRectNum = *pu32RectNum;
	*pu32RectNum = 0;
	//printf("------0000000000000------\n");

	u32BufLen = pstCatchYuvFrm->stVFrame.u32Width * pstCatchYuvFrm->stVFrame.u32Height * 3;
	iRet = HI_MPI_SYS_MmzAlloc_Cached(&u64Bgr888PicBufPhyAddr, 
			(HI_VOID**)&pcBgr888PicBufVirAddr, NULL, NULL, u32BufLen);
	if (HI_SUCCESS != iRet)
	{	
		printf("HI_MPI_SYS_MmzAlloc_Cached() failed.\n");
		goto EXIT;
	}
	iRet = ConvYuvFrmToBgr888(pstCatchYuvFrm, u64Bgr888PicBufPhyAddr, pcBgr888PicBufVirAddr);
	if (HI_SUCCESS != iRet)
	{
		printf("ConvYuvFrmToBgr888() failed.\n");
		goto EXIT_WITH_FREE;
	}
	HI_MPI_SYS_MmzFlushCache(u64Bgr888PicBufPhyAddr, pcBgr888PicBufVirAddr, u32BufLen);
	
	simage.data = (unsigned char *)pcBgr888PicBufVirAddr;
	simage.width = pstCatchYuvFrm->stVFrame.u32Width;
	simage.height = pstCatchYuvFrm->stVFrame.u32Height;
	simage.channels = 3;
	faces = hpstInstanceCtx->hpclFaceDetector->detect(simage);

	iDetectedFaceNum = (int)faces.size;
	if (iDetectedFaceNum <= 0)
	{
		//printf("size(%u, %u) iDetectedFaceNum = 0.\n", pstCatchYuvFrm->stVFrame.u32Width, pstCatchYuvFrm->stVFrame.u32Height);
		goto EXIT_WITH_FREE;
	}
	
	u32ObjRectNum = RT_MIN(iDetectedFaceNum, u32ObjRectNum);
	for (u32Idx = 0; u32Idx < u32ObjRectNum; ++u32Idx)
	{
	#if 1
		printf("\033[0;32m""ObjIdx(%u) stRect(%d, %d, %d, %d) Score(%d).\n""\033[0m", u32Idx, 
				faces.data[u32Idx].pos.x, faces.data[u32Idx].pos.y, 
				faces.data[u32Idx].pos.width, faces.data[u32Idx].pos.height, 
				(int)faces.data[u32Idx].score);
	#endif
		if ((int)faces.data[u32Idx].score < 100)
		{
			//continue ;
		}
		pstObjRect[*pu32RectNum].s32X = faces.data[u32Idx].pos.x;
		pstObjRect[*pu32RectNum].s32Y = faces.data[u32Idx].pos.y;
		pstObjRect[*pu32RectNum].u32Width = (HI_U32)faces.data[u32Idx].pos.width;
		pstObjRect[*pu32RectNum].u32Height = (HI_U32)faces.data[u32Idx].pos.height;
		//stResult.au32Score[u32Idx] = (HI_U32)(faces.data[u32Idx].score * 100);
		(*pu32RectNum)++;
	}

	//printf("------777777777777777777------\n");
	iRet = HI_SUCCESS;

EXIT_WITH_FREE:
	HI_MPI_SYS_MmzFree(u64Bgr888PicBufPhyAddr, pcBgr888PicBufVirAddr);
EXIT:
	return iRet;
}

int SEETA_FACE2_ADPT_DestroyInstance(RT_HANDLE hInstance)
{
    int iRet = HI_SUCCESS;
	SEETA_FACE2_INSTANCE_CTX_S *hpstInstanceCtx = (SEETA_FACE2_INSTANCE_CTX_S*)hInstance;

	delete hpstInstanceCtx->hpclFaceDetector;
	free(hpstInstanceCtx);
	
    return iRet;
}


