#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include <pthread.h>
#include <opencv2/core/core.hpp>
#include "chip_sdk_headers.h"
void DistanceMeasure(Mat *left,Mat *right,Mat *result)
{
    int iRet = HI_FAILURE;
    HI_U32 u32idx;
    HI_U64 u64Bgr888PicBufPhyAddr;
}