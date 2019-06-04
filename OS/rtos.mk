# FreeRTOS 编译
# Author：Virus.V
# Date:2019-06-02 16:06:05
ifeq ($(ROOT_DIR),)
$(error DO NOT Compile directly in this directory!)
endif
CURR_PATH := $(ROOT_DIR)/OS
RTOS_SRC_FILES = $(CURR_PATH)/list.c $(CURR_PATH)/queue.c $(CURR_PATH)/tasks.c
#RTOS_SRC_FILES = $(wildcard $(CURR_PATH)/*.c)
RTOS_SRC_FILES += $(wildcard $(CURR_PATH)/portable/GCC/ARM_CM3/*.c)
# 堆内存管理, 最简单的内存管理,不允许内存释放
RTOS_SRC_FILES += $(CURR_PATH)/portable/MemMang/heap_1.c

#include 目录
RTOS_INC_DIRS += $(CURR_PATH)/include $(CURR_PATH)/portable/GCC/ARM_CM3