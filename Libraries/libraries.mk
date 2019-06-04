# Stm32F10x标准库
ifeq ($(ROOT_DIR),)
$(error DO NOT Compile directly in this directoty!)
endif
CURR_PATH := $(ROOT_DIR)/Libraries
LIB_FILE_LIST := rcc gpio spi usart tim i2c iwdg dbgmcu pwr bkp rtc
#构造文件列表
LIBRARIES_SRC_FILES := $(CURR_PATH)/src/misc.c
LIBRARIES_SRC_FILES += $(addprefix $(CURR_PATH)/src/stm32f10x_,$(addsuffix .c,$(LIB_FILE_LIST)))
LIBRARIES_INC_DIRS := $(CURR_PATH)/inc
