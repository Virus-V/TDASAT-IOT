# CMSIS

ifeq ($(ROOT_DIR),)
$(error DO NOT Compile directly in this directory!)
endif
# 定义目录
CURR_PATH := $(ROOT_DIR)/CMSIS/CM3
SUB_DIRS := $(CURR_PATH)/CoreSupport $(CURR_PATH)/DeviceSupport/ST/STM32F10x
# 定义需要编译的源文件
# 
CMSIS_SRC_FILES := $(wildcard $(addsuffix /*.c,$(SUB_DIRS)))
CMSIS_INC_DIRS := $(SUB_DIRS)