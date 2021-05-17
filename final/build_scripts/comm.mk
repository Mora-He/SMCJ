CHIP ?= NOT_CARE
OS_KERNEL ?= linux
C_LIB ?= glibc 

ifneq ($(OS_KERNEL), liteos)
	CMP_FLAGS   += -fPIC
endif 
#CMP_FLAGS   += -Werror



ifeq ($(OS_KERNEL), liteos)   
    LITEOSTOPDIR=$(KERNEL_DIR)
    -include $(LITEOSTOPDIR)/config.mk
    CMP_FLAGS += $(LITEOS_CFLAGS) $(LITEOS_INCLUDE) $(LITEOS_USR_INCLUDE) -D__HuaweiLite__
endif
    
DEF_FLAGS   += -D_LARGEFILE64_SOURCE -D$(CHIP) -DOS_$(OS_KERNEL) -D${C_LIB} -D_REENTRANT

