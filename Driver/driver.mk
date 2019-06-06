# Driver
ifeq ($(ROOT_DIR),)
$(error DO NOT Compile directly in this directory!)
endif
CURR_PATH := $(ROOT_DIR)/Driver
#DRIVER_SRC_FILES := $(wildcard $(CURR_PATH)/*.c)
DRIVER_SRC_FILES := $(CURR_PATH)/OLED.c
DRIVER_INC_DIRS := $(CURR_PATH)