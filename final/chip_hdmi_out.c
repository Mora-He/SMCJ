#include "chip_sdk_headers.h"

static HI_VOID HISI_SAMPLE_COMM_VO_HdmiConvertSync(VO_INTF_SYNC_E enIntfSync,
    HI_HDMI_VIDEO_FMT_E *penVideoFmt)
{
    switch (enIntfSync)
    {
        case VO_OUTPUT_PAL:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_PAL;
            break;
        case VO_OUTPUT_NTSC:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_NTSC;
            break;
        case VO_OUTPUT_1080P24:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_24;
            break;
        case VO_OUTPUT_1080P25:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_25;
            break;
        case VO_OUTPUT_1080P30:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_30;
            break;
        case VO_OUTPUT_720P50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_720P_50;
            break;
        case VO_OUTPUT_720P60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_720P_60;
            break;
        case VO_OUTPUT_1080I50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080i_50;
            break;
        case VO_OUTPUT_1080I60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080i_60;
            break;
        case VO_OUTPUT_1080P50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_50;
            break;
        case VO_OUTPUT_1080P60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_60;
            break;
        case VO_OUTPUT_576P50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_576P_50;
            break;
        case VO_OUTPUT_480P60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_480P_60;
            break;
        case VO_OUTPUT_800x600_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_800X600_60;
            break;
        case VO_OUTPUT_1024x768_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1024X768_60;
            break;
        case VO_OUTPUT_1280x1024_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1280X1024_60;
            break;
        case VO_OUTPUT_1366x768_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1366X768_60;
            break;
        case VO_OUTPUT_1440x900_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1440X900_60;
            break;
        case VO_OUTPUT_1280x800_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1280X800_60;
            break;
        case VO_OUTPUT_1920x2160_30:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1920x2160_30;
            break;
        case VO_OUTPUT_1600x1200_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1600X1200_60;
            break;
        case VO_OUTPUT_1920x1200_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1920X1200_60;
            break;
        case VO_OUTPUT_2560x1440_30:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_2560x1440_30;
            break;
        case VO_OUTPUT_2560x1600_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_2560x1600_60;
            break;
        case VO_OUTPUT_3840x2160_30:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_3840X2160P_30;
            break;
        case VO_OUTPUT_3840x2160_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_3840X2160P_60;
        break;
        default:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_60;
            break;
    }

    return;
}

HI_S32 VO_HdmiStartByDyRg(VO_INTF_SYNC_E enIntfSync, DYNAMIC_RANGE_E enDyRg)
{
    HI_HDMI_ATTR_S          stAttr;
    HI_HDMI_VIDEO_FMT_E     enVideoFmt = HI_HDMI_VIDEO_FMT_BUTT;
    HI_HDMI_ID_E            enHdmiId    = HI_HDMI_ID_0;

    memset(&stAttr, 0, sizeof(HI_HDMI_ATTR_S));

    CHECK_RET(HI_MPI_HDMI_Init(), "HI_MPI_HDMI_Init");
    CHECK_RET(HI_MPI_HDMI_Open(enHdmiId), "HI_MPI_HDMI_Open");
    CHECK_RET(HI_MPI_HDMI_GetAttr(enHdmiId, &stAttr), "HI_MPI_HDMI_GetAttr");

	stAttr.bEnableHdmi           = HI_TRUE;
	stAttr.bEnableVideo          = HI_TRUE;
	stAttr.enVideoFmt            = HI_HDMI_VIDEO_FMT_1080P_60;
	stAttr.enVidOutMode          = HI_HDMI_VIDEO_MODE_RGB444;
	stAttr.enDeepColorMode       = HI_HDMI_DEEP_COLOR_24BIT;
	stAttr.bxvYCCMode            = HI_FALSE;
	stAttr.enOutCscQuantization  = HDMI_QUANTIZATION_LIMITED_RANGE;
	stAttr.bEnableAudio          = HI_FALSE;
	stAttr.enSoundIntf           = HI_HDMI_SND_INTERFACE_I2S;
	stAttr.bIsMultiChannel       = HI_FALSE;
	stAttr.enBitDepth            = HI_HDMI_BIT_DEPTH_16;
	stAttr.bEnableAviInfoFrame   = HI_TRUE;
	stAttr.bEnableAudInfoFrame   = HI_TRUE;
	stAttr.bEnableSpdInfoFrame   = HI_FALSE;
	stAttr.bEnableMpegInfoFrame  = HI_FALSE;
	stAttr.bDebugFlag            = HI_FALSE;
	stAttr.bHDCPEnable           = HI_FALSE;
	stAttr.b3DEnable             = HI_FALSE;
	stAttr.enDefaultMode         = HI_HDMI_FORCE_HDMI;	

    CHECK_RET(HI_MPI_HDMI_SetAttr(enHdmiId, &stAttr), "HI_MPI_HDMI_SetAttr");
    CHECK_RET(HI_MPI_HDMI_Start(enHdmiId), "HI_MPI_HDMI_Start");

    return HI_SUCCESS;
}


HI_S32 HISI_SAMPLE_COMM_VO_StartHdmi(VO_INTF_SYNC_E enIntfSync, DYNAMIC_RANGE_E enDyRg, 
		const AIO_ATTR_S *pstAioAttr)
{
    HI_HDMI_ATTR_S          stHdmiAttr;
    HI_HDMI_VIDEO_FMT_E     enVideoFmt = HI_HDMI_VIDEO_FMT_BUTT;
    HI_HDMI_ID_E            enHdmiId    = HI_HDMI_ID_0;

    memset(&stHdmiAttr, 0, sizeof(HI_HDMI_ATTR_S));

    HISI_SAMPLE_COMM_VO_HdmiConvertSync(enIntfSync, &enVideoFmt);

    CHECK_RET(HI_MPI_HDMI_Init(), "HI_MPI_HDMI_Init");
    CHECK_RET(HI_MPI_HDMI_Open(enHdmiId), "HI_MPI_HDMI_Open");
    CHECK_RET(HI_MPI_HDMI_GetAttr(enHdmiId, &stHdmiAttr), "HI_MPI_HDMI_GetAttr");
    stHdmiAttr.bEnableHdmi           = HI_TRUE;
    stHdmiAttr.bEnableVideo          = HI_TRUE;
    stHdmiAttr.enVideoFmt            = enVideoFmt;
    stHdmiAttr.enVidOutMode          = HI_HDMI_VIDEO_MODE_YCBCR444;
    switch(enDyRg)
    {
        case DYNAMIC_RANGE_SDR8:
            stHdmiAttr.enDeepColorMode = HI_HDMI_DEEP_COLOR_24BIT;
            break;
        case DYNAMIC_RANGE_HDR10:
            stHdmiAttr.enVidOutMode    = HI_HDMI_VIDEO_MODE_YCBCR422;
            break;
        default:
            stHdmiAttr.enDeepColorMode = HI_HDMI_DEEP_COLOR_24BIT;
            break;
    }
    stHdmiAttr.bxvYCCMode            = HI_FALSE;
    stHdmiAttr.enOutCscQuantization  = HDMI_QUANTIZATION_LIMITED_RANGE;

    stHdmiAttr.bEnableAudio          = HI_FALSE;
    stHdmiAttr.enSoundIntf           = HI_HDMI_SND_INTERFACE_I2S;
    stHdmiAttr.bIsMultiChannel       = HI_FALSE;

    stHdmiAttr.enBitDepth            = HI_HDMI_BIT_DEPTH_16;

    stHdmiAttr.bEnableAviInfoFrame   = HI_TRUE;
    stHdmiAttr.bEnableAudInfoFrame   = HI_TRUE;
    stHdmiAttr.bEnableSpdInfoFrame   = HI_FALSE;
    stHdmiAttr.bEnableMpegInfoFrame  = HI_FALSE;

    stHdmiAttr.bDebugFlag            = HI_FALSE;
    stHdmiAttr.bHDCPEnable           = HI_FALSE;

    stHdmiAttr.b3DEnable             = HI_FALSE;
    stHdmiAttr.enDefaultMode         = HI_HDMI_FORCE_HDMI;

	if (NULL != pstAioAttr)
	{
		stHdmiAttr.bEnableAudio = HI_TRUE;		  /**< if enable audio */
		stHdmiAttr.enSoundIntf = HI_HDMI_SND_INTERFACE_I2S; /**< source of HDMI audio, HI_HDMI_SND_INTERFACE_I2S suggested.the parameter must be consistent with the input of AO*/
		stHdmiAttr.enSampleRate = (HI_HDMI_SAMPLE_RATE_E)pstAioAttr->enSamplerate; 	   /**< sampling rate of PCM audio,the parameter must be consistent with the input of AO */
		stHdmiAttr.u8DownSampleParm = HI_FALSE;    /**< parameter of downsampling  rate of PCM audio, default :0 */

		stHdmiAttr.enBitDepth = (HI_HDMI_BIT_DEPTH_E)(8 * (pstAioAttr->enBitwidth + 1));   /**< bitwidth of audio,default :16,the parameter must be consistent with the config of AO */
		stHdmiAttr.u8I2SCtlVbit = 0;		/**< reserved, should be 0, I2S control (0x7A:0x1D) */

		stHdmiAttr.bEnableAviInfoFrame = HI_TRUE; /**< if enable  AVI InfoFrame*/
		stHdmiAttr.bEnableAudInfoFrame = HI_TRUE; /**< if enable AUDIO InfoFrame*/
	}

    CHECK_RET(HI_MPI_HDMI_SetAttr(enHdmiId, &stHdmiAttr), "HI_MPI_HDMI_SetAttr");
    CHECK_RET(HI_MPI_HDMI_Start(enHdmiId), "HI_MPI_HDMI_Start");

    return HI_SUCCESS;
}

HI_S32 HISI_SAMPLE_COMM_VO_StopHdmi(HI_VOID)
{
    HI_HDMI_ID_E enHdmiId = HI_HDMI_ID_0;

    CHECK_RET(HI_MPI_HDMI_Stop(enHdmiId),"HI_MPI_HDMI_Stop");
    CHECK_RET(HI_MPI_HDMI_Close(enHdmiId),"HI_MPI_HDMI_Close");
    CHECK_RET(HI_MPI_HDMI_DeInit(),"HI_MPI_HDMI_DeInit");

    return HI_SUCCESS;
}

HI_S32 HISI_SAMPLE_COMM_SetHdmiAudioAttr(const AIO_ATTR_S *pstAioAttr)
{
    HI_S32 s32Ret;
    HI_HDMI_ATTR_S stHdmiAttr;
    HI_HDMI_ID_E enHdmi = HI_HDMI_ID_0;

	s32Ret = HI_MPI_HDMI_Stop(enHdmi);
	if(HI_SUCCESS != s32Ret)
	{
		printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_HDMI_GetAttr(enHdmi, &stHdmiAttr);
	if(HI_SUCCESS != s32Ret)
	{
		printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
		return HI_FAILURE;
	}

	stHdmiAttr.bEnableAudio = HI_TRUE;		  /**< if enable audio */
	stHdmiAttr.enSoundIntf = HI_HDMI_SND_INTERFACE_I2S; /**< source of HDMI audio, HI_HDMI_SND_INTERFACE_I2S suggested.the parameter must be consistent with the input of AO*/
	stHdmiAttr.enSampleRate = (HI_HDMI_SAMPLE_RATE_E)pstAioAttr->enSamplerate; 	   /**< sampling rate of PCM audio,the parameter must be consistent with the input of AO */
	stHdmiAttr.u8DownSampleParm = HI_FALSE;    /**< parameter of downsampling  rate of PCM audio, default :0 */

	stHdmiAttr.enBitDepth = (HI_HDMI_BIT_DEPTH_E)(8 * (pstAioAttr->enBitwidth + 1));   /**< bitwidth of audio,default :16,the parameter must be consistent with the config of AO */
	stHdmiAttr.u8I2SCtlVbit = 0;		/**< reserved, should be 0, I2S control (0x7A:0x1D) */

	stHdmiAttr.bEnableAviInfoFrame = HI_TRUE; /**< if enable c  AVI InfoFrame*/
	stHdmiAttr.bEnableAudInfoFrame = HI_TRUE; /**< if enable AUDIO InfoFrame*/

	s32Ret = HI_MPI_HDMI_SetAttr(enHdmi, &stHdmiAttr);
	if(HI_SUCCESS != s32Ret)
	{
		printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_HDMI_Start(enHdmi);
	if(HI_SUCCESS != s32Ret)
	{
		printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
		return HI_FAILURE;
	}

	return s32Ret;
}

int HISI_SAMPLE_COMM_SetHdmiAudioMute(HI_BOOL bMute)
{
    HI_S32 s32Ret;
    HI_HDMI_ATTR_S stHdmiAttr;
    HI_HDMI_ID_E enHdmi = HI_HDMI_ID_0;

	s32Ret = HI_MPI_HDMI_Stop(enHdmi);
	if(HI_SUCCESS != s32Ret)
	{
		printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_HDMI_GetAttr(enHdmi, &stHdmiAttr);
	if(HI_SUCCESS != s32Ret)
	{
		printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
		return HI_FAILURE;
	}

	if (bMute)
	{
		stHdmiAttr.bEnableAudio = HI_FALSE;
	}
	else
	{
		stHdmiAttr.bEnableAudio = HI_TRUE;		  /**< if enable audio */
	}
	s32Ret = HI_MPI_HDMI_SetAttr(enHdmi, &stHdmiAttr);
	if(HI_SUCCESS != s32Ret)
	{
		printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_HDMI_Start(enHdmi);
	if(HI_SUCCESS != s32Ret)
	{
		printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
		return HI_FAILURE;
	}

	return s32Ret;
}

