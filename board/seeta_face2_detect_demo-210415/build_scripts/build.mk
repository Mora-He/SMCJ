CC          = $(CROSS_PREFIX)gcc
CPP         = $(CROSS_PREFIX)g++
AR          = $(CROSS_PREFIX)ar 
STRIP       = $(CROSS_PREFIX)strip
LD          = $(CROSS_PREFIX)ld
OBJCOPY		= $(CROSS_PREFIX)objcopy

CMP_FLAGS   += -Wall -ffunction-sections -fdata-sections -Wl,-Bsymbolic
CMP_FLAGS   += -fstack-protector-all -D_REENTRANT
#CMP_FLAGS   += -fstack-protector-strong 	#Can't do this 
 
ifeq ($(DEBUG), y)
    CMP_FLAGS   += -ggdb
endif

CPP_FLAGS   += -fpermissive

SRC_FILES   += $(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.c))
SRC_FILES   += $(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.cpp))
OBJS_TO_BUILD += $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SRC_FILES)))
#OBJS_TO_RM += $(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.o))

INC_FLAGS   += $(addprefix -I, $(INC_DIR))
LD_FLAGS   += $(addprefix -L, $(MEDIA_LIBS_RELEASE_DEST_DIR))
ifeq ($(OS_KERNEL), liteos)
	APP_BIN     := $(TARGET).liteos
else
	LD_FLAGS   += -Wl,-gc-sections
	APP_BIN     := $(TARGET)
endif

STATIC_LIB  := lib$(TARGET).a
SHARED_LIB  := lib$(TARGET).so


APP_RELEASE_DIR := ~/$(CHIP)


.c.o:
    ifneq ($(USE_C_COMPILER), n)
	    $(CC)  $(CMP_FLAGS) $(DEF_FLAGS) $(FEATURES_FLAGS) $(INC_FLAGS) -c  -o $@ $<
    else
	    $(CPP) $(CPP_FLAGS) $(CMP_FLAGS) $(DEF_FLAGS) $(FEATURES_FLAGS) $(INC_FLAGS) -c  -o $@ $<
    endif
    
.cpp.o:
	$(CPP) $(CPP_FLAGS) $(CMP_FLAGS) $(DEF_FLAGS) $(FEATURES_FLAGS) $(INC_FLAGS) -c -o $@ $<


bin:$(OBJS_TO_BUILD) 
    ifeq ($(OS_KERNEL), liteos)
	    $(LD) $(LD_FLAGS) -o $(TARGET) $^  $(DEPEND_OBJS) -lgcc -lgcc_eh -lcpup -lsupc++ -lc -lcsysdeps -lsec -lm -lposix -llinuxadp -lvfs -llwip -lgpio -li2c -lmem -lrtc -luart -lpm -lshell -ltelnet -lcortex-a17 -lhi3559   -lipcm -lipcm_net -lsharefs -lcppsupport -lstdc++ -ldynload -lz -lproc -llwip -lhidmac -lrandom -lspi -luart -lpm -lbspcommon -lgcc
	    $(OBJCOPY) -O binary $(TARGET) $(APP_BIN)
    else
	    echo $(MEDIA_LIBS_RELEASE_DEST_DIR)
    	ifneq ($(USE_C_COMPILER), n)
	    $(CC) -o $(APP_BIN) -Xlinker "-(" $^ $(DEPEND_OBJS) $(LD_FLAGS) $(POST_LD_FLAGS) $(APP_PRIVATE_LD_FLAGS) -Xlinker "-)" 
    	else
	    $(CPP) -o $(APP_BIN) -Xlinker "-(" $^ $(DEPEND_OBJS) $(LD_FLAGS) $(POST_LD_FLAGS) $(APP_PRIVATE_LD_FLAGS) -Xlinker "-)"     
    	endif 
    endif

    ifneq ($(DEBUG), y)
	    $(STRIP) $(APP_BIN)
    endif
    ifneq ($(APP_RELEASE_DIR), )
	    @mkdir -p $(APP_RELEASE_DIR)
	    cp -f $(APP_BIN) $(APP_RELEASE_DIR)/

    endif
    
	@echo "------------------------------------------------------------------------------------------------------------------"
	@echo ""
	@echo "                    Congradulations Build $(APP_BIN) Successfully for $(CHIP)"
	@echo ""
	@echo "------------------------------------------------------------------------------------------------------------------"    
	
	    
lib: $(OBJS_TO_BUILD) 
ifneq ($(OBJS_TO_BUILD), )
    ifneq ($(MEDIA_LIBS_RELEASE_DEST_DIR), )
	    @mkdir -p $(MEDIA_LIBS_RELEASE_DEST_DIR)
    endif
	$(AR) -rcs $(MEDIA_LIBS_RELEASE_DEST_DIR)/$(STATIC_LIB) $^
endif


so: $(OBJS_TO_BUILD) 
ifneq ($(OBJS_TO_BUILD), )
    ifneq ($(MEDIA_LIBS_RELEASE_DEST_DIR), )
	    @mkdir -p $(MEDIA_LIBS_RELEASE_DEST_DIR)
    endif
    ifneq ($(USE_C_COMPILER), n)
	    $(CC) -shared -fPIC -o $(MEDIA_LIBS_RELEASE_DEST_DIR)/$(SHARED_LIB) $^ 
    else
	    $(CPP) -shared -fPIC -o $(MEDIA_LIBS_RELEASE_DEST_DIR)/$(SHARED_LIB) $^ 
    endif
endif


clean:
	-rm -f  $(OBJS_TO_BUILD)  $(APP_BIN)
	-rm -rf  $(MEDIA_LIBS_RELEASE_DEST_DIR)/$(STATIC_LIB) 
	-rm -rf  $(MEDIA_LIBS_RELEASE_DEST_DIR)/$(SHARED_LIB) 
 

