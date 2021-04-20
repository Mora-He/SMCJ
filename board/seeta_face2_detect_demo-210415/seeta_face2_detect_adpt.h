#ifndef __SEETA_FACE2_DETECT_DEMO__
#define __SEETA_FACE2_DETECT_DEMO__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "chip_sdk_headers.h"

typedef void* RT_HANDLE;


int SEETA_FACE2_ADPT_CreateInstance(RT_HANDLE *phInstance);

int SEETA_FACE2_ADPT_SendPic(RT_HANDLE hInstance, const VIDEO_FRAME_INFO_S *pstCatchYuvFrm, 
		RECT_S *pstObjRect, HI_U32 *pu32RectNum);

int SEETA_FACE2_ADPT_DestroyInstance(RT_HANDLE hInstance);

#endif