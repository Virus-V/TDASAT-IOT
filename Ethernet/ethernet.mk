# Ethernet 
ifeq ($(ROOT_DIR),)
$(error DO NOT Compile directly in this directory!)
endif
CURR_PATH := $(ROOT_DIR)/Ethernet
ETHERNET_SRC_FILES := $(wildcard $(CURR_PATH)/*.c $(CURR_PATH)/W5500/*.c)
ETHERNET_INC_DIRS := $(CURR_PATH) $(CURR_PATH)/W5500