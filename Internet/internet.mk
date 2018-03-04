# Internet 协议
ifeq ($(ROOT_DIR),)
$(error DO NOT Compile directly in this directory!)
endif
CURR_PATH := $(ROOT_DIR)/Internet
SUB_DIRS := $(CURR_PATH)/DHCP $(CURR_PATH)/DNS
INTERNET_SRC_FILES := $(wildcard $(addsuffix /*.c,$(SUB_DIRS)))
INTERNET_INC_DIRS := $(ROOT_DIR)/Internet