#ifndef __CHIP_SDK_HEADER_H__
#define __CHIP_SDK_HEADER_H__

#include <syslog.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pwd.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>
#include <dirent.h>
#include <pthread.h>

#include <limits.h>

#include <signal.h>

#include <sys/reboot.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <linux/reboot.h>
//#include <linux/kernel.h>	/* conflicted with <sys/sysinfo.h> */
#include <linux/rtc.h>
#include <time.h>
#include <termios.h>
#include <byteswap.h>

#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/vfs.h>
#include <sys/prctl.h>
#include <sys/mount.h>
#include <mntent.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/epoll.h>
#include <linux/falloc.h>

#include <arpa/inet.h>
#include <sys/socket.h>	   
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/route.h>

#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <regex.h>


#include "hi_common.h"
#include "hi_buffer.h"
#include "hi_comm_sys.h"
#include "hi_comm_vb.h"
#include "hi_comm_isp.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "hi_comm_venc.h"
#include "hi_comm_vdec.h"
#include "hi_comm_vpss.h"
#include "hi_comm_avs.h"
#include "hi_comm_region.h"
#include "hi_comm_adec.h"
#include "hi_comm_aenc.h"
#include "hi_comm_ai.h"
#include "hi_comm_ao.h"
#include "hi_comm_aio.h"
#include "hi_defines.h"
#include "hi_comm_hdmi.h"
#include "hi_mipi.h"
#include "hi_comm_vgs.h"
#include "hi_comm_ive.h"

#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"
#include "mpi_vdec.h"
#include "mpi_vpss.h"
#include "mpi_avs.h"
#include "mpi_region.h"
#include "mpi_audio.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "hi_math.h"
#include "hi_sns_ctrl.h"
#include "mpi_hdmi.h"
#include "mpi_vgs.h"
#include "mpi_ive.h"

#include "hifb.h"
// #include "fb.h"


#define MIPI_DEV_NODE       "/dev/hi_mipi"

#define RT_MIN(a, b)    (((a) < (b)) ? (a) : (b))

#define HI_ERR_PRINT printf

static VI_DEV_ATTR_S DEV_ATTR_MIPI_BASE =	//DEV_ATTR_IMX290_2M_BASE
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFF00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    {-1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1280,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            720,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {1920, 1080},
    {
        {
            {1920 , 1080},

        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1080
    },
    DATA_RATE_X1
};


static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX290_12BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3, -1, -1, -1, -1}
        }
    }
};


static VI_PIPE_ATTR_S PIPE_ATTR_1920x1080_RAW12_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    1920, 1080,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_12,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};


#define CHECK_RET(express,name)\
    do{\
        HI_S32 Ret;\
        Ret = express;\
        if (HI_SUCCESS != Ret)\
        {\
            printf("\033[0;31m%s failed at %s: LINE: %d with %#x!\033[0;39m\n", name, __FUNCTION__, __LINE__, Ret);\
            return Ret;\
        }\
    }while(0)


#endif
