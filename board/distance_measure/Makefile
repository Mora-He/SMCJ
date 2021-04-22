CUR_DIR     = $(shell pwd)

#CHIP?=HI3519V10X
CHIP?=HI3519AV100
#CHIP?=SSC339G

ifeq ($(CHIP), HI3519V10X)
CROSS_PREFIX=arm-hisiv500-linux-
SDK_VERSION=
else ifeq ($(CHIP), HI3519AV100)
CROSS_PREFIX=arm-himix200-linux-
SDK_VERSION=
else ifeq ($(CHIP), SSC339G)
CROSS_PREFIX=arm-linux-gnueabihf-9.1.0-
SDK_VERSION=release_618
endif

BUILD_SCRIPT_DIR=$(CUR_DIR)/build_scripts

MEDIA_LIBS_RELEASE_DEST_DIR=$(CUR_DIR)/../lib/


include $(BUILD_SCRIPT_DIR)/comm.mk

# TARGET = seeta_face2_detect_demo
TARGET = distance_measure

USE_C_COMPILER=n

DEBUG = n

SDK_DIR=$(CUR_DIR)/../..
MPP_DIR=mpp_smp_linux

INC_DIR += $(SDK_DIR)/$(MPP_DIR)/include  $(SDK_DIR)/drv/interdrv/rtc  $(SDK_DIR)/drv/interdrv/wdt  $(SDK_DIR)/$(MPP_DIR)/component/isp/ext_drv/hi3519av100/pwm

SRC_DIR += $(CUR_DIR) 

INTERFACE_LIB_DIR=$(CUR_DIR)/SeetaFace2/using-package
LD_FLAGS   += $(foreach dir, $(INTERFACE_LIB_DIR)/FaceDetector, $(wildcard $(dir)/*.a))
INC_DIR += $(INTERFACE_LIB_DIR)/FaceDetector/include/seeta

LD_FLAGS   += $(foreach dir, $(MEDIA_LIBS_RELEASE_DEST_DIR), $(wildcard $(dir)/lib*.a))
#LD_FLAGS   += $(foreach dir, $(MEDIA_LIBS_RELEASE_DEST_DIR), $(wildcard $(dir)/lib*.so*))

LD_FLAGS            += -lpthread -lm
LD_FLAGS            += -ldl
LD_FLAGS            += -lresolv -lrt 
#LD_FLAGS            += -static

#CMP_FLAGS   += -Werror -O3


include $(BUILD_SCRIPT_DIR)/build.mk


