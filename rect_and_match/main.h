#ifndef __DPU_MAIN_H__
#define __DPU_MAIN_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#include "sample_comm.h"    // ??????????????

/* This case only for function design reference */
HI_S32 DPU_VI_VPSS_RECT_MATCH(HI_VOID);
HI_VOID DPU_VI_VPSS_RECT_MATCH_HandleSig(HI_VOID);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SAMPLE_DPU_MAIN_H__ */
