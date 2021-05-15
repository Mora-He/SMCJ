#include "main.h"

static char **s_ppChCmdArgv = NULL;

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
#ifndef __HuaweiLite__
void DPU_HandleSig(int s32Signo)
{
    signal(SIGINT,SIG_IGN);
    signal(SIGTERM,SIG_IGN);

    if (SIGINT == s32Signo || SIGTERM == s32Signo)
    {
		switch (*s_ppChCmdArgv[1])
	    {
            case '0':
            {
                DPU_VI_VPSS_RECT_MATCH_HandleSig();
                break;
            }
            default :
            {
                break;
            }
	    }

        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}
#endif

/******************************************************************************
* function : show usage
******************************************************************************/
void DPU_Usage(char* pchPrgName)
{
    printf("Usage : %s <index> \n", pchPrgName);
    printf("index:\n");
    printf("\t 0) VI->VPSS->RECT->MATCH.\n");
    printf("\t 1) FILE->RECT->MATCH.\n");
}

/******************************************************************************
* function : ive sample
******************************************************************************/
#ifdef __HuaweiLite__
int app_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    int s32Ret = HI_SUCCESS;

    if (argc < 2)
    {
        DPU_Usage(argv[0]);
        return HI_FAILURE;
    }
	s_ppChCmdArgv = argv;
#ifndef __HuaweiLite__
    signal(SIGINT, DPU_HandleSig);
    signal(SIGTERM, DPU_HandleSig);
#endif

    if (*argv[1] == '0')
    {
        s32Ret = DPU_VI_VPSS_RECT_MATCH();   // 调用VI_PASS矫正和匹配
    }

    if (HI_SUCCESS == s32Ret)
    {
        SAMPLE_PRT("program exit normally!\n");
    }
    else
    {
        SAMPLE_PRT("program exit abnormally!\n");
    }

    return s32Ret;
}
    