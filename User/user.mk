# TDSAST_IOT 应用实现是
ifeq ($(ROOT_DIR),)
$(error DO NOT Compile directly in this directory!)
endif
CURR_PATH := $(ROOT_DIR)/User
USER_SRC_FILES := $(wildcard $(CURR_PATH)/*.c)
USER_INC_DIRS := $(CURR_PATH)