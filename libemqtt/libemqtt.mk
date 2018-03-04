# Embedded MQTT 协议
ifeq ($(ROOT_DIR),)
$(error DO NOT Compile directly in this directory!)
endif
CURR_PATH := $(ROOT_DIR)/libemqtt
LIBEMQTT_SRC_FILES := $(CURR_PATH)/src/libemqtt.c
LIBEMQTT_INC_DIRS := $(CURR_PATH)/include