#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include <pthread.h>
#include <opencv2/core/core.hpp>
#include "chip_sdk_headers.h"
#include "seeta_face2_detect_adpt.h"

#define VIDEO_INPUT_CHN_NUM	2

#define VIDEO_INPUT_WIDTH	1920
#define VIDEO_INPUT_HEIGHT	1080

#define SEETA_FACE2_PROC_WIDTH	640
#define SEETA_FACE2_PROC_HEIGHT	480



typedef struct
{
	bool bContinue;
	RGN_HANDLE hOsdRgn;
    BITMAP_S stOsdBitmap;
}DEMO_MNG_CTX_S;

static DEMO_MNG_CTX_S s_stDemoMngCtx;
static int s_hfdMipi = -1;


static HI_S32 HISI_SAMPLE_COMM_VI_EnableMipiClock(void)
{
    HI_S32 iRet = HI_SUCCESS;
    combo_dev_t devno;
	
	for (devno = 0; devno < MIPI_RX_MAX_DEV_NUM; ++devno)
	{
		iRet = ioctl(s_hfdMipi, HI_MIPI_ENABLE_MIPI_CLOCK, &devno);
	}
	
EXIT:
	return iRet;
}

static HI_S32 HISI_SAMPLE_COMM_VI_ResetMipi(void)
{
    HI_S32 iRet = HI_SUCCESS;
    combo_dev_t devno;
	
	for (devno = 0; devno < MIPI_RX_MAX_DEV_NUM; ++devno)
	{
		iRet = ioctl(s_hfdMipi, HI_MIPI_RESET_MIPI, &devno);
	}
	
EXIT:
	return iRet;
}

static HI_S32 HISI_SAMPLE_COMM_VI_UnresetMipi(void)
{
    HI_S32 iRet = HI_SUCCESS;
    combo_dev_t devno;
	
	for (devno = 0; devno < MIPI_RX_MAX_DEV_NUM; ++devno)
	{
		iRet = ioctl(s_hfdMipi, HI_MIPI_UNRESET_MIPI, &devno);
	}
	
EXIT:
	return iRet;
}

static HI_S32 HISI_SAMPLE_COMM_VI_EnableSensorClock(void)
{
    HI_S32 iRet = HI_SUCCESS;
    sns_rst_source_t SnsDev;

    for (SnsDev = 0; SnsDev < SNS_MAX_CLK_SOURCE_NUM; SnsDev++)
    {
        iRet = ioctl(s_hfdMipi, HI_MIPI_ENABLE_SENSOR_CLOCK, &SnsDev);
        if (HI_SUCCESS != iRet)
        {
            HI_ERR_PRINT("HI_MIPI_ENABLE_SENSOR_CLOCK failed\n");
            goto EXIT;
        }
    }

EXIT:
    return iRet;
}

static HI_S32 HISI_SAMPLE_COMM_VI_ResetSensor(void)
{
    HI_S32 iRet = HI_SUCCESS;
    sns_rst_source_t SnsDev = 0;

    for (SnsDev = 0; SnsDev < SNS_MAX_RST_SOURCE_NUM; SnsDev++)
    {
        iRet = ioctl(s_hfdMipi, HI_MIPI_RESET_SENSOR, &SnsDev);
        if (HI_SUCCESS != iRet)
        {
            HI_ERR_PRINT("HI_MIPI_RESET_SENSOR failed\n");
            goto EXIT;
        }
    }

EXIT:
    return iRet;
}

static HI_S32 HISI_SAMPLE_COMM_VI_UnresetSensor(void)
{
    HI_S32 iRet = HI_SUCCESS;
    sns_rst_source_t SnsDev = 0;

    for (SnsDev = 0; SnsDev < SNS_MAX_RST_SOURCE_NUM; SnsDev++)
    {
        iRet = ioctl(s_hfdMipi, HI_MIPI_UNRESET_SENSOR, &SnsDev);
        if (HI_SUCCESS != iRet)
        {
            HI_ERR_PRINT("HI_MIPI_UNRESET_SENSOR failed\n");
            goto EXIT;
        }
    }

EXIT:
    return iRet;
}

static int DEMO_InitSys(const VB_CONFIG_S *pstVbConfig, lane_divide_mode_t enHsMode)
{
	int iRet;
    int i;
	
    HI_MPI_SYS_Exit();
    for( i = 0; i < VB_MAX_POOLS; i++)
    {
         HI_MPI_VB_DestroyPool(i);
    }
    HI_MPI_VB_Exit();

    iRet = HI_MPI_VB_SetConfig(pstVbConfig);
    if (HI_SUCCESS != iRet)
    {
        printf("HI_MPI_VB_SetConfig() failed!\n");
        //goto EXIT;
    }
	//VIC_TODO: HI_MPI_VB_SetSupplementConfig();
    iRet = HI_MPI_VB_Init();
    if (HI_SUCCESS != iRet)
    {
        printf("HI_MPI_VB_Init() failed!\n");
        goto EXIT;
    }
#if 0
    iRet = HI_MPI_SYS_SetConfig(&pstHisiAvAttr->stSysConf);
    if (HI_SUCCESS != iRet)
    {
        printf("HI_MPI_SYS_SetConfig() failed\n");
        goto EXIT;
    }
#endif
    iRet = HI_MPI_SYS_Init();
    if (HI_SUCCESS != iRet)
    {
        printf("HI_MPI_SYS_Init() failed!\n");
        goto EXIT;
    }

    s_hfdMipi = open(MIPI_DEV_NODE, O_RDWR);
    if (s_hfdMipi < 0)
    {
        HI_ERR_PRINT("open hi_mipi dev failed\n");
        goto EXIT;
    }

    iRet = ioctl(s_hfdMipi, HI_MIPI_SET_HS_MODE, &enHsMode);
    if (HI_SUCCESS != iRet)
    {
        HI_ERR_PRINT("HI_MIPI_SET_HS_MODE failed\n");
		goto EXIT;
    }
	
	HISI_SAMPLE_COMM_VI_EnableMipiClock();
	HISI_SAMPLE_COMM_VI_ResetMipi();

	HISI_SAMPLE_COMM_VI_EnableSensorClock();
	HISI_SAMPLE_COMM_VI_ResetSensor();

EXIT:
	return iRet;
}

static void SetVpssModParam(void)
{
	VPSS_MOD_PARAM_S stModParam;

	if (HI_SUCCESS == HI_MPI_VPSS_GetModParam(&stModParam))
	{
		stModParam.bOneBufForLowDelay = HI_FALSE;
		stModParam.u32VpssVbSource = VB_SOURCE_COMMON;
		HI_MPI_VPSS_SetModParam(&stModParam);
	}

	return ;
}

static HI_S32 HISI_SAMPLE_COMM_VI_StartMIPI(const combo_dev_attr_t *pstComboDevAttr, 
        const phy_cmv_t *pstPhyCmv)
{   
    int iRet = HI_FAILURE;
	sns_rst_source_t SnsDev;

    if (NULL == pstComboDevAttr)
    {
        goto EXIT;
    }
	#if 0
    HI_FMT_PRINT("devno(%u) input_mode(%u) data_rate(%u).\n", 
		    pstComboDevAttr->devno, pstComboDevAttr->input_mode, pstComboDevAttr->data_rate);
	#endif

	SnsDev = pstComboDevAttr->devno;
#if defined HI3519AV100 || (defined AMP_HI3519AV100)
	SnsDev = (pstComboDevAttr->devno + 1 ) / 2;
	if (INPUT_MODE_SLVS == pstComboDevAttr->input_mode)
	{
		iRet = ioctl(s_hfdMipi, HI_MIPI_ENABLE_SLVS_CLOCK, &pstComboDevAttr->devno);
		iRet = ioctl(s_hfdMipi, HI_MIPI_RESET_SLVS, &pstComboDevAttr->devno);
		
		iRet = ioctl(s_hfdMipi, HI_MIPI_ENABLE_SENSOR_CLOCK, &SnsDev);
		iRet = ioctl(s_hfdMipi, HI_MIPI_UNRESET_SENSOR, &SnsDev);
	}
#else
	iRet = ioctl(s_hfdMipi, HI_MIPI_ENABLE_MIPI_CLOCK, &SnsDev);
	iRet = ioctl(s_hfdMipi, HI_MIPI_RESET_MIPI, &SnsDev);
	iRet = ioctl(s_hfdMipi, HI_MIPI_ENABLE_SENSOR_CLOCK, &SnsDev);
	iRet = ioctl(s_hfdMipi, HI_MIPI_RESET_SENSOR, &SnsDev);
#endif

	iRet = ioctl(s_hfdMipi, HI_MIPI_SET_DEV_ATTR, pstComboDevAttr);
    if (HI_SUCCESS != iRet)
    {
        printf("set mipi attr failed with 0x%x.\n", iRet);
        goto EXIT;
    }

#if defined HI3519AV100 || (defined AMP_HI3519AV100)
	if (INPUT_MODE_SLVS == pstComboDevAttr->input_mode)
	{
		iRet = ioctl(s_hfdMipi, HI_MIPI_UNRESET_SLVS, &pstComboDevAttr->devno);
	}
	else
#endif
	{
		iRet = ioctl(s_hfdMipi, HI_MIPI_UNRESET_MIPI, &pstComboDevAttr->devno);
	}

	iRet = ioctl(s_hfdMipi, HI_MIPI_UNRESET_SENSOR, &SnsDev);
	if (HI_SUCCESS != iRet)
	{
		printf("HI_MIPI_UNRESET_SENSOR(%d) failed.\n", SnsDev);
		//goto EXIT;
	}

    if (NULL != pstPhyCmv)
    {
        if (ioctl(s_hfdMipi, HI_MIPI_SET_PHY_CMVMODE, pstPhyCmv))
        {
            printf("set mipi phy common voltage mode failed\n");
            goto EXIT;
        }    
    }

    iRet = HI_SUCCESS;
    
EXIT:    
    return iRet;
}

static HI_S32 HISI_SAMPLE_COMM_VI_StartIsp(VI_PIPE ViPipe, const ISP_SNS_OBJ_S *pstSnsObj, 
        ISP_SNS_COMMBUS_U unSnsBusInfo, bool bOnlyToSwitchWorkMode, const ISP_PUB_ATTR_S *pstPubAttr)
{
    HI_S32 iRet;
    ALG_LIB_S stAeLib;
    ALG_LIB_S stAwbLib;
    //ALG_LIB_S stAfLib;
	
	if (NULL == pstSnsObj)
	{
	    goto EXIT;
	}

#if 0   
    /* 0. set cmos iniparser file path */
    iRet = sensor_set_inifile_path("configs/");
    if (iRet != HI_SUCCESS)
    {
        printf("%s: set cmos iniparser file path failed with %#x!\n", \
               __FUNCTION__, iRet);
        return iRet;
    }
#endif

    stAeLib.s32Id = ViPipe;
    stAwbLib.s32Id = ViPipe;
    //stAfLib.s32Id = ViPipe;
    strncpy(stAeLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    strncpy(stAwbLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    //strncpy(stAfLib.acLibName, HI_AF_LIB_NAME, sizeof(HI_AF_LIB_NAME)); 
    if (pstSnsObj->pfnRegisterCallback != HI_NULL)
    {
        iRet = pstSnsObj->pfnRegisterCallback(ViPipe, &stAeLib, &stAwbLib);
        if (iRet != HI_SUCCESS)
        {
            printf("%s: sensor_register_callback failed with %#x!\n", __FUNCTION__, iRet);
            return iRet;
        }
    }
    else
    {
        printf("%s: sensor_register_callback failed with HI_NULL!\n",  __FUNCTION__);
    }

    if (HI_NULL == pstSnsObj->pfnSetBusInfo)
    {
        printf("pstSnsObj->pfnSetBusInfo is NULL!\n");
        return HI_FAILURE;
	}
    iRet = pstSnsObj->pfnSetBusInfo(ViPipe, unSnsBusInfo);
    if (iRet != HI_SUCCESS)
    {
        printf("set sensor bus info failed with %#x!\n", iRet);
        return iRet;
    }

    iRet = HI_MPI_AE_Register(ViPipe, &stAeLib);
    if (iRet != HI_SUCCESS)
    {
        printf("%s: HI_MPI_AE_Register failed!\n", __FUNCTION__);
        return iRet;
    }

    iRet = HI_MPI_AWB_Register(ViPipe, &stAwbLib);
    if (iRet != HI_SUCCESS)
    {
        printf("%s: HI_MPI_AWB_Register failed!\n", __FUNCTION__);
        return iRet;
    }

#ifdef USE_HISI_AF_LIB
    iRet = HI_MPI_AF_Register(ViPipe, &stAfLib);
    if (iRet != HI_SUCCESS)
    {
        printf("%s: HI_MPI_AF_Register failed!\n", __FUNCTION__);
        return iRet;
    }
#else
	//iRet = RegisterAfLib(ViPipe);
#endif

    iRet = HI_MPI_ISP_MemInit(ViPipe);
    if (iRet != HI_SUCCESS)
    {
        printf("%s: HI_MPI_ISP_MemInit failed!\n", __FUNCTION__);
        return iRet;
    }

    iRet = HI_MPI_ISP_SetPubAttr(ViPipe, pstPubAttr);
    if (iRet != HI_SUCCESS)
    {
        printf("%s: HI_MPI_ISP_SetPubAttr failed with %#x!\n", __FUNCTION__, iRet);
        return iRet;
    }

	if (!bOnlyToSwitchWorkMode)
	{
	    /* 8. isp init */
	    iRet = HI_MPI_ISP_Init(ViPipe);
	    if (iRet != HI_SUCCESS)
	    {
	        printf("%s: HI_MPI_ISP_Init failed!\n", __FUNCTION__);
	        return iRet;
	    }
	}
	else
	{
	    ISP_INNER_STATE_INFO_S stInnerStateInfo;
	    while (1)
	    {
	        HI_MPI_ISP_QueryInnerStateInfo(ViPipe, &stInnerStateInfo);
	        if ((HI_TRUE == stInnerStateInfo.bResSwitchFinish) || (HI_TRUE == stInnerStateInfo.bWDRSwitchFinish))
	        {
	            break;
	        }
	        usleep(1000);
	    }
	}

EXIT:
    return HI_SUCCESS;
}

static int StartSensor(int iIdx)
{
	int iRet;
    const ISP_SNS_OBJ_S *pstSnsObj;	
    ISP_PUB_ATTR_S stIspPubAttr;
    combo_dev_attr_t *pstComboDevAttr = NULL;	
    VI_DEV ViDev = iIdx;
	VI_DEV_ATTR_S stViDevAttr;
	VI_CHN ViChn = 0;
	VI_CHN_ATTR_S stViChnAttr;
    MIPI_DEV MipiDev;
	VI_DEV_BIND_PIPE_S stDevBindPipe;
	VI_PIPE ViPipe = ViDev;
	VI_PIPE_ATTR_S stPipeAttr;
	ISP_SNS_TYPE_E enSensorAccessIntfType = ISP_SNS_I2C_TYPE;
    ISP_SNS_COMMBUS_U unSnsBusInfo = {0};
	int i;
	
	memcpy(&stPipeAttr, &PIPE_ATTR_1920x1080_RAW12_420_3DNR_RFR, sizeof(stPipeAttr));
	pstComboDevAttr = &MIPI_4lane_CHN0_SENSOR_IMX290_12BIT_2M_NOWDR_ATTR;
	pstSnsObj = &stSnsImx290Obj;
	memcpy(&stViDevAttr, &DEV_ATTR_MIPI_BASE, sizeof(stViDevAttr));
    memset(&stIspPubAttr, 0, sizeof(stIspPubAttr));

	for (i = 0; i < MIPI_LANE_NUM; ++i)
	{
		pstComboDevAttr->mipi_attr.lane_id[i] = -1;
	}

	switch (iIdx)
	{
		case 0:
		{
			pstComboDevAttr->mipi_attr.lane_id[0] = 0;
			pstComboDevAttr->mipi_attr.lane_id[1] = 1;
			pstComboDevAttr->mipi_attr.lane_id[2] = 2;
			pstComboDevAttr->mipi_attr.lane_id[3] = 3;
			unSnsBusInfo.s8I2cDev = 1;
			break;
		}
		case 1:
		{
			pstComboDevAttr->mipi_attr.lane_id[0] = 4;
			pstComboDevAttr->mipi_attr.lane_id[1] = 5;
			pstComboDevAttr->mipi_attr.lane_id[2] = 6;
			pstComboDevAttr->mipi_attr.lane_id[3] = 7;
			unSnsBusInfo.s8I2cDev = 3;
			break;
		}
		case 2:
		{
			pstComboDevAttr->mipi_attr.lane_id[0] = 8;
			pstComboDevAttr->mipi_attr.lane_id[1] = 9;
			pstComboDevAttr->mipi_attr.lane_id[2] = 10;
			pstComboDevAttr->mipi_attr.lane_id[3] = 11;
			unSnsBusInfo.s8I2cDev = 5;
			break;
		}
		default :
		{
			goto EXIT;
		}
	}

	MipiDev = ViDev;
	pstComboDevAttr->devno = MipiDev;

	iRet = HISI_SAMPLE_COMM_VI_StartMIPI(pstComboDevAttr, NULL);
    stViDevAttr.stSize.u32Width = VIDEO_INPUT_WIDTH;
    stViDevAttr.stSize.u32Height = VIDEO_INPUT_HEIGHT;
    stViDevAttr.stBasAttr.stSacleAttr.stBasSize.u32Width  = VIDEO_INPUT_WIDTH;
    stViDevAttr.stBasAttr.stSacleAttr.stBasSize.u32Height = VIDEO_INPUT_HEIGHT;
    //stViDevAttr.stWDRAttr.enWDRMode = ;
	iRet = HI_MPI_VI_SetDevAttr(ViDev, &stViDevAttr);
    iRet = HI_MPI_VI_EnableDev(ViDev);

	//BindViDevAndPipe
	stDevBindPipe.u32Num = 1;
	stDevBindPipe.PipeId[0] = ViPipe;
    iRet = HI_MPI_VI_SetDevBindPipe(ViDev, &stDevBindPipe);
	
	stPipeAttr.enBitWidth = DATA_BITWIDTH_8;
	stPipeAttr.u32MaxW = VIDEO_INPUT_WIDTH;
	stPipeAttr.u32MaxH = VIDEO_INPUT_HEIGHT;
	stPipeAttr.enCompressMode = COMPRESS_MODE_NONE;
    iRet = HI_MPI_VI_CreatePipe(ViPipe, &stPipeAttr);
	iRet = HI_MPI_VI_StartPipe(ViPipe);

    memset(&stViChnAttr, 0, sizeof(stViChnAttr));
	stViChnAttr.stSize.u32Width = VIDEO_INPUT_WIDTH;
	stViChnAttr.stSize.u32Height = VIDEO_INPUT_HEIGHT;
    stViChnAttr.enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stViChnAttr.stFrameRate.s32SrcFrameRate = -1;    
    stViChnAttr.stFrameRate.s32DstFrameRate = stViChnAttr.stFrameRate.s32SrcFrameRate;
    iRet = HI_MPI_VI_SetChnAttr(ViPipe, ViChn, &stViChnAttr);
	iRet = HI_MPI_VI_EnableChn(ViPipe, ViChn);

	stIspPubAttr.enBayer = BAYER_RGGB;
    stIspPubAttr.f32FrameRate = 30;
	stIspPubAttr.stWndRect.u32Width	= VIDEO_INPUT_WIDTH;
	stIspPubAttr.stWndRect.u32Height	= VIDEO_INPUT_HEIGHT;
	stIspPubAttr.stSnsSize.u32Width	= VIDEO_INPUT_WIDTH;
	stIspPubAttr.stSnsSize.u32Height	= VIDEO_INPUT_HEIGHT;
	iRet = HISI_SAMPLE_COMM_VI_StartIsp(ViPipe, pstSnsObj, unSnsBusInfo, false, &stIspPubAttr);

EXIT:
	return iRet;
}

int Hisi_StartVpssGrp(VPSS_GRP VpssGrp, HI_U32 u32Width, HI_U32 u32Height, PIXEL_FORMAT_E enPixelFormat,
		DYNAMIC_RANGE_E enDynamicRange, VPSS_NR_TYPE_E enNrType, COMPRESS_MODE_E enNrRefFrmCompressMode)
{
    int s32Ret = HI_FAILURE;
    
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    memset(&stVpssGrpAttr, 0, sizeof(stVpssGrpAttr));
    stVpssGrpAttr.u32MaxW = u32Width;
    stVpssGrpAttr.u32MaxH = u32Height;
    stVpssGrpAttr.enPixelFormat = enPixelFormat;     /* RW; Pixel format of source image. */
    stVpssGrpAttr.enDynamicRange = enDynamicRange;    /* RW; DynamicRange of source image. */
    stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;       /* Grp frame rate contrl. */
    stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;       /* Grp frame rate contrl. */
	if (enNrType < VPSS_NR_TYPE_BUTT)
	{
	    stVpssGrpAttr.bNrEn = HI_TRUE;
	}
	else
	{
	    stVpssGrpAttr.bNrEn = HI_FALSE;
	}
	stVpssGrpAttr.stNrAttr.enNrType = enNrType;
	stVpssGrpAttr.stNrAttr.enCompressMode = enNrRefFrmCompressMode;   /* RW; Reference frame compress mode */
	stVpssGrpAttr.stNrAttr.enNrMotionMode = NR_MOTION_MODE_NORMAL;	//VIC_TODO
    s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
    if (s32Ret != HI_SUCCESS)
    {
        HI_ERR_PRINT("HI_MPI_VPSS_CreateGrp(%d) failed with 0x%x!\n", VpssGrp, s32Ret);
        goto EXIT;
    }
	s32Ret = HI_MPI_VPSS_ResetGrp(VpssGrp);
	if (s32Ret != HI_SUCCESS)
	{
		goto EXIT;
	}

	#if defined HI3516EV300 || (defined LITEOS_HI3516EV300) || defined HI3516DV300 || defined AMP_HI3559V200
	FRAME_INTERRUPT_ATTR_S stFrameIntAttr;
	s32Ret = HI_MPI_VPSS_GetGrpFrameInterruptAttr(VpssGrp, &stFrameIntAttr);
    if (s32Ret == HI_SUCCESS)
	{
		stFrameIntAttr.enIntType = FRAME_INTERRUPT_EARLY_END;
		stFrameIntAttr.u32EarlyLine = 200;	//VIC_TODO:
		HI_MPI_VPSS_SetGrpFrameInterruptAttr(VpssGrp, &stFrameIntAttr);
	}
	#endif

    s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        HI_MPI_VPSS_DestroyGrp(VpssGrp);
        HI_ERR_PRINT("HI_MPI_VPSS_StartGrp() failed with %#x\n", s32Ret);
        goto EXIT;
    }
	
    s32Ret = HI_SUCCESS;

EXIT:
    return s32Ret;
}

int Hisi_StopVpssGrp(VPSS_GRP VpssGrp)
{
    int s32Ret = HI_FAILURE;
	int VpssChn;

	for (VpssChn = VPSS_MAX_CHN_NUM - 1; VpssChn >= 0; --VpssChn)
	{
		HI_MPI_VPSS_DisableChn(VpssGrp, VpssChn);
	}
	HI_MPI_VPSS_ResetGrp(VpssGrp);
    HI_MPI_VPSS_StopGrp(VpssGrp);
    HI_MPI_VPSS_DestroyGrp(VpssGrp);
    
    s32Ret = HI_SUCCESS;

//EXIT:
    return s32Ret;
}

int Hisi_EnableVpssChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CHN_ATTR_S *pstChnAttr)
{
    int s32Ret = HI_FAILURE;

    if (NULL != pstChnAttr)
    {
        s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, pstChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            HI_ERR_PRINT("HI_MPI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
            goto EXIT;
        }
    }

    s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
    if (s32Ret != HI_SUCCESS)
    {
        HI_ERR_PRINT("HI_MPI_VPSS_EnableChn failed with %#x\n", s32Ret);
        goto EXIT;
    }

    s32Ret = HI_SUCCESS;

EXIT:
    return s32Ret;
}

int Hisi_Bind(MOD_ID_E enSrcMod, HI_S32 s32SrcDev, HI_S32 s32SrcChn, 
        MOD_ID_E enDstMod, HI_S32 s32DstDev, HI_S32 s32DstChn)
{
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    stSrcChn.enModId = enSrcMod;
    stSrcChn.s32DevId = s32SrcDev;
    stSrcChn.s32ChnId = s32SrcChn;

    stDestChn.enModId = enDstMod;
    stDestChn.s32DevId = s32DstDev;
    stDestChn.s32ChnId = s32DstChn;
    return HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
}


HI_S32 HISI_SAMPLE_COMM_VO_StartHdmi(VO_INTF_SYNC_E enIntfSync, DYNAMIC_RANGE_E enDyRg, 
		const AIO_ATTR_S *pstAioAttr);

static int StartHdmiOutput(VO_INTF_SYNC_E enIntfSync)
{
    int iRet = HI_FAILURE;
    VO_DEV VoPhyDev = 0;
    VO_PUB_ATTR_S stVoPubAttr;
    VO_LAYER VoLayer = 0;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    VO_CSC_S stVideoCSC;
	AIO_ATTR_S stAoAttr;
	AIO_ATTR_S *pstAoAttr = NULL;
	DYNAMIC_RANGE_E enDstDyRg = DYNAMIC_RANGE_SDR8;
	VO_CHN_ATTR_S stVoChnAttr;
	VO_CHN VoChn = 0;

    memset(&stVoPubAttr, 0, sizeof(stVoPubAttr));
	stVoPubAttr.u32BgColor = 0x00ff00;
    stVoPubAttr.enIntfType = VO_INTF_HDMI;
	stVoPubAttr.enIntfSync = enIntfSync;	
    iRet = HI_MPI_VO_SetPubAttr(VoPhyDev, &stVoPubAttr);
    if (HI_SUCCESS != iRet)
    {
        HI_ERR_PRINT("HI_MPI_VO_SetPubAttr(%d) failed with 0x%x\n", VoPhyDev, iRet);
        goto EXIT;
    }
	
    iRet = HI_MPI_VO_Enable(VoPhyDev);
    if (HI_SUCCESS != iRet)
    {
        HI_ERR_PRINT("HI_MPI_VO_Enable(%d) failed with 0x%x\n", VoPhyDev, iRet);
        goto EXIT;
    }

	if (0 != (stVoPubAttr.enIntfType & VO_INTF_HDMI))
	{
		#if 0
		iRet = HI_MPI_AO_GetPubAttr(HISI_HDMI_AO_DEV_ID, &stAoAttr);
		if (HI_SUCCESS == iRet)
		{
			pstAoAttr = &stAoAttr;
		}
		else
		{
			pstAoAttr = NULL;
		}
		#endif
		HISI_SAMPLE_COMM_VO_StartHdmi(stVoPubAttr.enIntfSync, enDstDyRg, pstAoAttr);
	}

    VoLayer = VoPhyDev;
    HI_MPI_VO_SetDisplayBufLen(VoLayer, 4);
	memset(&stLayerAttr, 0, sizeof(stLayerAttr));
    stLayerAttr.stDispRect.s32X = 0;
    stLayerAttr.stDispRect.s32Y = 0;
    stLayerAttr.stDispRect.u32Width = 1920;
    stLayerAttr.stDispRect.u32Height = 1080;
    stLayerAttr.stImageSize.u32Width = stLayerAttr.stDispRect.u32Width;	/* Canvas size of the video layer */
    stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;	/* Canvas size of the video layer */
    stLayerAttr.u32DispFrmRt = 60;                /* Display frame rate */
    stLayerAttr.enPixFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;         /* Pixel format of the video layer */
    stLayerAttr.bDoubleFrame = HI_FALSE;               /* Whether to double frames */  
    stLayerAttr.bClusterMode = HI_FALSE;	//VIC_TODO
	stLayerAttr.enDstDynamicRange = enDstDyRg;
    iRet = HI_MPI_VO_SetVideoLayerAttr(VoLayer, &stLayerAttr);
    if (HI_SUCCESS != iRet)
    {
        HI_ERR_PRINT("HI_MPI_VO_SetVideoLayerAttr(%d) failed with 0x%x\n", VoLayer, iRet);
        goto EXIT;
    }

    iRet = HI_MPI_VO_EnableVideoLayer(VoLayer);
    if (HI_SUCCESS != iRet)
    {
        HI_ERR_PRINT("HI_MPI_VO_EnableVideoLayer(%d) failed with 0x%x\n", VoLayer, iRet);
        goto EXIT;
    }

    stVideoCSC.enCscMatrix = VO_CSC_MATRIX_IDENTITY;
    stVideoCSC.u32Luma = 50;                     /* luminance:   0 ~ 100 default: 50 */
    stVideoCSC.u32Contrast = 50;                 /* contrast :   0 ~ 100 default: 50 */
    stVideoCSC.u32Hue = 50;                      /* hue      :   0 ~ 100 default: 50 */
    stVideoCSC.u32Satuature = 50;                /* satuature:   0 ~ 100 default: 50 */
    (void)HI_MPI_VO_SetVideoLayerCSC(VoLayer, &stVideoCSC);

	memset(&stVoChnAttr, 0, sizeof(stVoChnAttr));
	memcpy(&stVoChnAttr.stRect, &stLayerAttr.stDispRect, sizeof(stVoChnAttr.stRect));
	iRet = HI_MPI_VO_SetChnAttr(VoLayer, VoChn, &stVoChnAttr);
	if (HI_SUCCESS != iRet)
	{
		HI_ERR_PRINT("HI_MPI_VO_SetChnAttr(%d, %d) failed with 0x%x\n", VoLayer, VoChn, iRet);
        goto EXIT;
	}
	iRet = HI_MPI_VO_EnableChn(VoLayer, VoChn);
	if (HI_SUCCESS != iRet)
	{
		HI_ERR_PRINT("HI_MPI_VO_EnableChn(%d, %d) failed with 0x%x\n", VoLayer, VoChn, iRet);
        goto EXIT;
	}			 
	
EXIT:
    return iRet;
}

static void StartObjOsd()
{
	int iRet;
	RGN_ATTR_S stRgnAttr;
	RGN_CHN_ATTR_S stOsdChnAttr;
    MPP_CHN_S stMppChn;

	s_stDemoMngCtx.hOsdRgn = 0;

	memset(&s_stDemoMngCtx.stOsdBitmap, 0, sizeof(s_stDemoMngCtx.stOsdBitmap));
    s_stDemoMngCtx.stOsdBitmap.u32Width = VIDEO_INPUT_WIDTH;
    s_stDemoMngCtx.stOsdBitmap.u32Height = VIDEO_INPUT_HEIGHT;
    s_stDemoMngCtx.stOsdBitmap.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	s_stDemoMngCtx.stOsdBitmap.pData = \
			malloc(s_stDemoMngCtx.stOsdBitmap.u32Width * s_stDemoMngCtx.stOsdBitmap.u32Height * 2);
	if (NULL == s_stDemoMngCtx.stOsdBitmap.pData)
	{
		goto EXIT;
	}
	
	memset(&stRgnAttr, 0, sizeof(stRgnAttr));
    stRgnAttr.enType = OVERLAYEX_RGN;
    stRgnAttr.unAttr.stOverlayEx.enPixelFmt = PIXEL_FORMAT_ARGB_1555;
    stRgnAttr.unAttr.stOverlayEx.u32BgColor = 0;        
    stRgnAttr.unAttr.stOverlayEx.stSize.u32Width = VIDEO_INPUT_WIDTH;
    stRgnAttr.unAttr.stOverlayEx.stSize.u32Height = VIDEO_INPUT_HEIGHT;
	stRgnAttr.unAttr.stOverlayEx.u32CanvasNum = 2;
    iRet = HI_MPI_RGN_Create(s_stDemoMngCtx.hOsdRgn, &stRgnAttr);
    if (HI_SUCCESS != iRet)
    {
        printf("HI_MPI_RGN_Create() failed with %#x!\n", iRet);
        goto EXIT;
    }

	memset(s_stDemoMngCtx.stOsdBitmap.pData, 0, 
			s_stDemoMngCtx.stOsdBitmap.u32Width * s_stDemoMngCtx.stOsdBitmap.u32Height * 2);
    iRet = HI_MPI_RGN_SetBitMap(s_stDemoMngCtx.hOsdRgn, &s_stDemoMngCtx.stOsdBitmap);
    if (HI_SUCCESS != iRet)
    {
        printf("HI_MPI_RGN_SetBitMap() failed with %#x!\n", iRet);
        goto EXIT;
    }

    memset(&stOsdChnAttr, 0, sizeof(stOsdChnAttr));
    stOsdChnAttr.bShow = HI_TRUE;
    stOsdChnAttr.enType = OVERLAYEX_RGN;
    stOsdChnAttr.unChnAttr.stOverlayChn.u32BgAlpha = 0;
    stOsdChnAttr.unChnAttr.stOverlayChn.u32FgAlpha = 127;
    stOsdChnAttr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
    stOsdChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp = 0;
    stOsdChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = HI_FALSE;
    stOsdChnAttr.unChnAttr.stOverlayChn.u32Layer = 2;	
    stMppChn.enModId  = HI_ID_VPSS;
    stMppChn.s32DevId = 0;
    stMppChn.s32ChnId = 0;
    iRet = HI_MPI_RGN_AttachToChn(s_stDemoMngCtx.hOsdRgn, &stMppChn, &stOsdChnAttr);
    if (HI_SUCCESS != iRet)
    {
        printf("HI_MPI_RGN_AttachToChn() failed with %#x!\n", iRet);
        goto EXIT;
    }

EXIT:
	return iRet;
}

static void DrawRectForArgb1555PackedPic(const BITMAP_S *pstBitmap, const RECT_S *pstRect, HI_U32 u32BorderWidth,
		HI_U16 u16ColorVal)
{
	HI_U32 u32RowIdx, u32ColIdx;
	HI_U16 *pu16Tmp;
	HI_U32 i;
		
	pu16Tmp = (HI_U16*)((char*)pstBitmap->pData + pstBitmap->u32Width * pstRect->s32Y * 2 + pstRect->s32X * 2);
	for (u32ColIdx = 0; u32ColIdx < pstRect->u32Width; ++u32ColIdx)
	{
		for (i = 0; i < u32BorderWidth; ++i)
		{
			*(pu16Tmp + u32ColIdx + (pstBitmap->u32Width * i)) = u16ColorVal;	//Top line
			
			*(pu16Tmp + u32ColIdx + pstBitmap->u32Width * (pstRect->u32Height - 1 - i)) = u16ColorVal;	//Bottom line
		}
	}

	for (u32RowIdx = 1; u32RowIdx < (pstRect->u32Height - 1); ++u32RowIdx)
	{
		for (i = 0; i < u32BorderWidth; ++i)
		{
			*(pu16Tmp + pstBitmap->u32Width * u32RowIdx + i) = u16ColorVal;	//Left line
			
			*(pu16Tmp + pstBitmap->u32Width * u32RowIdx + pstRect->u32Width - 1 - i) = u16ColorVal;	//Right line
		}
	}

    return ;
}

static void UpdateObjOsd(const RECT_S *pstObjRect, HI_U32 u32RectNum)
{
	int iRet;
	HI_U32 u32Idx;

	memset(s_stDemoMngCtx.stOsdBitmap.pData, 0, 
			s_stDemoMngCtx.stOsdBitmap.u32Width * s_stDemoMngCtx.stOsdBitmap.u32Height * 2);
	for (u32Idx = 0; u32Idx < u32RectNum; ++u32Idx)
	{
		DrawRectForArgb1555PackedPic(&s_stDemoMngCtx.stOsdBitmap, &pstObjRect[u32Idx], 2, 0x801f);
	}
    iRet = HI_MPI_RGN_SetBitMap(s_stDemoMngCtx.hOsdRgn, &s_stDemoMngCtx.stOsdBitmap);
    if (HI_SUCCESS != iRet)
    {
        printf("HI_MPI_RGN_SetBitMap() failed with %#x!\n", iRet);
        goto EXIT;
    }

EXIT:
	return iRet;
}

static void StopObjOsd()
{
    MPP_CHN_S stMppChn;

    stMppChn.enModId  = HI_ID_VPSS;
    stMppChn.s32DevId = 0;
    stMppChn.s32ChnId = 0;

	HI_MPI_RGN_DetachFromChn(s_stDemoMngCtx.hOsdRgn, &stMppChn);
	HI_MPI_RGN_Destroy(s_stDemoMngCtx.hOsdRgn);

	free(s_stDemoMngCtx.stOsdBitmap.pData);
	
	return ;
}


int main(int argc, char *argv[])
{
    int	iRet = EXIT_FAILURE;
	VB_CONFIG_S stVbConfig;
	lane_divide_mode_t enHsMode = LANE_DIVIDE_MODE_4;
	VI_VPSS_MODE_S stViVpssMode;
	HI_U32 i;
	VPSS_CHN_ATTR_S stVpssChnAttr;
    VIDEO_FRAME_INFO_S stCatchYuvFrm;
	RT_HANDLE hAlgInstance;
	RECT_S astObjRect[64];
	HI_U32 u32RectNum;

	memset(&stVbConfig, 0, sizeof(stVbConfig));
	stVbConfig.u32MaxPoolCnt = 128;
	stVbConfig.astCommPool[0].u64BlkSize = COMMON_GetPicBufferSize(VIDEO_INPUT_WIDTH, VIDEO_INPUT_HEIGHT, 
	        PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, 64);
	stVbConfig.astCommPool[0].u32BlkCnt = 10 * VIDEO_INPUT_CHN_NUM;
    iRet = DEMO_InitSys(&stVbConfig, enHsMode);
    if (HI_SUCCESS != iRet)
	{
        printf("DEMO_InitSys() failed!\n");
		goto EXIT;
	}

	memset(&stViVpssMode, 0, sizeof(stViVpssMode));
	iRet = HI_MPI_SYS_SetVIVPSSMode(&stViVpssMode);
	if (HI_SUCCESS != iRet)
	{
		printf("HI_MPI_SYS_SetVIVPSSMode() failed!\n");
		goto EXIT;
	}

	SetVpssModParam();
	printf("\033[0;33m""Func[%s] Line[%d]: s_iCounter(%d)\n""\033[0m", __FUNCTION__, __LINE__, 0);

	memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
	stVpssChnAttr.enChnMode = VPSS_CHN_MODE_USER;
	stVpssChnAttr.enVideoFormat = VIDEO_FORMAT_LINEAR;	  /* RW; Video format of target image. */
	stVpssChnAttr.enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;		/* RW; Pixel format of target image. */
	stVpssChnAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;	   /* RW; DynamicRange of target image. */
	stVpssChnAttr.enCompressMode = COMPRESS_MODE_NONE;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.stAspectRatio.enMode = ASPECT_RATIO_NONE; 	 /* Aspect Ratio info. */

	for (i = 0; i < VIDEO_INPUT_CHN_NUM; ++i)
	{
		iRet = StartSensor(i);
		
		iRet = Hisi_StartVpssGrp(i, VIDEO_INPUT_WIDTH, VIDEO_INPUT_HEIGHT, PIXEL_FORMAT_YVU_SEMIPLANAR_420, 
				DYNAMIC_RANGE_SDR8, VPSS_NR_TYPE_VIDEO, COMPRESS_MODE_NONE);
		
		stVpssChnAttr.u32Width = VIDEO_INPUT_WIDTH;
		stVpssChnAttr.u32Height = VIDEO_INPUT_HEIGHT;
		stVpssChnAttr.u32Depth = 0; 		  /* RW; Range: [0, 8]; User get list depth. */
		iRet = Hisi_EnableVpssChn(i, 0, &stVpssChnAttr);
		if (HI_SUCCESS != iRet)
		{
			printf("Hisi_EnableVpssChn(%d, 0) failed!\n", i);
		}
		stVpssChnAttr.u32Width = SEETA_FACE2_PROC_WIDTH;
		stVpssChnAttr.u32Height = SEETA_FACE2_PROC_HEIGHT;
		stVpssChnAttr.u32Depth = 2; 		  /* RW; Range: [0, 8]; User get list depth. */
		iRet = Hisi_EnableVpssChn(i, 1, &stVpssChnAttr);
		if (HI_SUCCESS != iRet)
		{
			printf("Hisi_EnableVpssChn(%d, 0) failed!\n", i);
		}

		Hisi_Bind(HI_ID_VI, i, 0, HI_ID_VPSS, i, 0);
	}

	StartHdmiOutput(VO_OUTPUT_1080P60);
	printf("\033[0;33m""Func[%s] Line[%d]: s_iCounter(%d)\n""\033[0m", __FUNCTION__, __LINE__, 6);
	Hisi_Bind(HI_ID_VPSS, 0, 0, HI_ID_VO, 0, 0);
    
    Hisi_Bind(HI_ID_VPSS, 1, 0, HI_ID_VO, 1, 0);

	iRet = SEETA_FACE2_ADPT_CreateInstance(&hAlgInstance);
	if (HI_SUCCESS != iRet)
	{
		printf("SEETA_FACE2_ADPT_CreateInstance() failed!\n");
		goto EXIT;
	}
	StartObjOsd();
	s_stDemoMngCtx.bContinue = true;
    
    // *********************** 此处为自己替换的变量 *********************

    // *********************** 结束 ********************************
	while (s_stDemoMngCtx.bContinue)
	{	
        iRet = HI_MPI_VPSS_GetChnFrame(0, 1, &stCatchYuvFrm, 300);
		if (HI_SUCCESS != iRet)
		{
			continue;
		}

		u32RectNum = sizeof(astObjRect) / sizeof(astObjRect[0]);

        // ********************** 此处换为自己的测距代码 *****************************

		SEETA_FACE2_ADPT_SendPic(hAlgInstance, &stCatchYuvFrm, astObjRect, &u32RectNum);
        // ********************** 结束 *********************************************

		HI_MPI_VPSS_ReleaseChnFrame(0, 1, &stCatchYuvFrm);

		for (i = 0; i < u32RectNum; ++i)
		{
		#if 0
			printf("\t Rect[%u] = {%d, %d, %u, %u}.\n", i, 
					astObjRect[i].s32X, astObjRect[i].s32Y, astObjRect[i].u32Width, astObjRect[i].u32Height);
		#else
			astObjRect[i].s32X = astObjRect[i].s32X * VIDEO_INPUT_WIDTH / stCatchYuvFrm.stVFrame.u32Width;
			astObjRect[i].s32Y = astObjRect[i].s32Y * VIDEO_INPUT_HEIGHT / stCatchYuvFrm.stVFrame.u32Height;
			astObjRect[i].u32Width = astObjRect[i].u32Width * VIDEO_INPUT_WIDTH / stCatchYuvFrm.stVFrame.u32Width;
			astObjRect[i].u32Height = astObjRect[i].u32Height * VIDEO_INPUT_HEIGHT / stCatchYuvFrm.stVFrame.u32Height;
		#endif
		}

		UpdateObjOsd(astObjRect, u32RectNum);
	}
	StopObjOsd();
	iRet = SEETA_FACE2_ADPT_DestroyInstance(hAlgInstance);

	//VIC_TODO: release resource.

    iRet = EXIT_SUCCESS;

EXIT:
    return iRet;
}
