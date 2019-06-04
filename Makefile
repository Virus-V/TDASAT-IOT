# TDSAST-IOT
# Author：Virus.V
# Date：2017-07-07 09:22:03
# Copyright：NJUPT-TDSAST

ROOT_DIR := $(shell pwd)
BIN_DIR = $(ROOT_DIR)/debug
#目标
TARGET = $(BIN_DIR)/sast_iot
#链接器脚本文件
LINKER_SCRIPT = $(ROOT_DIR)/Misc/flash.ld
STARTUP_FILE = $(ROOT_DIR)/Misc/startup_stm32f10x_md.s
#c_only_startup.s 
#startup_stm32f10x_md.s

include $(ROOT_DIR)/CMSIS/cmsis.mk
include $(ROOT_DIR)/Driver/driver.mk
include $(ROOT_DIR)/Ethernet/ethernet.mk
include $(ROOT_DIR)/Internet/internet.mk
include $(ROOT_DIR)/libemqtt/libemqtt.mk
include $(ROOT_DIR)/Libraries/libraries.mk
include $(ROOT_DIR)/User/user.mk
include $(ROOT_DIR)/OS/rtos.mk

ALL_SRC_FILE = $(CMSIS_SRC_FILES) $(DRIVER_SRC_FILES) $(ETHERNET_SRC_FILES) \
				$(INTERNET_SRC_FILES) $(LIBRARIES_SRC_FILES) $(USER_SRC_FILES) \
				$(LIBEMQTT_SRC_FILES) $(RTOS_SRC_FILES)
		
ALL_INC_PATH = $(CMSIS_INC_DIRS) $(DRIVER_INC_DIRS) $(ETHERNET_INC_DIRS) \
				$(INTERNET_INC_DIRS) $(LIBRARIES_INC_DIRS) $(USER_INC_DIRS) \
				$(LIBEMQTT_INC_DIRS) $(RTOS_INC_DIRS)
		
ALL_LIB_PATH = $(CMSIS_LIB_DIRS) 
ALL_OBJ_FILE = $(subst .c,.o,$(ALL_SRC_FILE)) $(subst .s,.o,$(STARTUP_FILE))


VPATH = $(ALL_INC_PATH) $(ALL_LIB_PATH)

# MCU name and submodel
MCU      = cortex-m3
SUBMDL   = stm32f103

# toolchain (using code sourcery now)
TCHAIN = arm-none-eabi
THUMB    = -mthumb
THUMB_IW = -mthumb-interwork

# Define programs and commands.
SHELL = sh
CC = $(TCHAIN)-gcc
CPP = $(TCHAIN)-g++
AR = $(TCHAIN)-ar
OBJCOPY = $(TCHAIN)-objcopy
OBJDUMP = $(TCHAIN)-objdump
SIZE = $(TCHAIN)-size
NM = $(TCHAIN)-nm
REMOVE = rm -f
REMOVEDIR = rm -r
COPY = cp

# Optimization level [0,1,2,3,s]
OPT ?= 0
#DEBUG = 
DEBUG = -ggdb

#宏定义
DEFINES = -D STM32F10X_MD -D USE_STDPERIPH_DRIVER

CFLAGS = $(DEBUG)
CFLAGS += -O$(OPT)
CFLAGS += -ffunction-sections -fdata-sections 
#-Wall -w 关闭所有
CFLAGS += -Wall -Wimplicit -Wno-pointer-sign
CFLAGS += -Wno-cast-align
CFLAGS += -Wpointer-arith -Wswitch
CFLAGS += -nostartfiles
CFLAGS += -mlittle-endian
CFLAGS += -Wredundant-decls -Wreturn-type -Wshadow -Wunused
#CFLAGS += -Wa,-adhlns=$(OBJS_DIR)/$$(subst $$(suffix $$<),.lst,$$(notdir $$<))
CFLAGS += -Wa,-adhlns=$(subst $(suffix $<),.lst,$<)
# 条件编译，选择dsp库和器件类型
CFLAGS += $(DEFINES)
# Aeembler Flags
#ASFLAGS = -Wa,-adhlns=$(OBJS_DIR)/$$(subst .s,.lst,$$(notdir $$<)) #,--g$(DEBUG)
ASFLAGS = -Wa,-adhlns=$(subst .s,.lst,$<) #,--g$(DEBUG)
# Linker Flags
LDFLAGS = -nostartfiles -Wl,-Map=$(TARGET).map,--cref,--gc-sections
LDFLAGS += $(addprefix -L ,$(ALL_LIB_PATH))
#-larm_cortexM4lf_math 
LDFLAGS += -lgcc -lc
LDFLAGS += --specs=nano.specs --specs=nosys.specs -Wl,--no-wchar-size-warning
#-u_printf_float --specs=nosys.specs -Wl,--no-wchar-size-warning,--undefined=uxTopUsedPriority
# Set the linker script
#LDFLAGS +=-T$(ROOT_DIR)/Assembly/c_only_md.ld
LDFLAGS += -T$(LINKER_SCRIPT)
# Combine all necessary flags and optional flags.
# Add target processor to flags. softfp
# -mfpu=fpv4-sp-d16 -mfloat-abi=hard
ALL_CFLAGS  = -mcpu=$(MCU) $(THUMB_IW) -I. $(addprefix -I ,$(ALL_INC_PATH)) $(CFLAGS)
ALL_ASFLAGS = -mcpu=$(MCU) $(THUMB_IW) -I. -x assembler-with-cpp $(ASFLAGS)

HEXSIZE = $(SIZE) --target=binary $(TARGET).hex
ELFSIZE = $(SIZE) -A $(TARGET).elf

#导出所有变量
#export

all: build 

build: elf bin lss sym sizeafter end
#build: elf bin sym sizeafter end

bin: $(TARGET).bin
elf: $(TARGET).elf
lss: $(TARGET).lss
sym: $(TARGET).sym

sizeafter:
	@if [ -f $(TARGET).elf ]; then echo "Size after"; $(ELFSIZE); fi

end:
	@echo "Make Complete~"
	@echo $(shell date)

# Create final output file (.hex) from ELF output file.
%.hex: %.elf
	$(OBJCOPY) -O binary $< $@

# Create final output file (.bin) from ELF output file.
%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

# Create extended listing file from ELF output file.
# testing: option -C
%.lss: %.elf
	$(OBJDUMP) -h -S -D $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	$(NM) -n $< > $@

# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(ALL_OBJ_FILE)

%.elf: $(ALL_OBJ_FILE)
	@$(CC) $(THUMB) $(ALL_CFLAGS) $(ALL_OBJ_FILE) --output $@ $(LDFLAGS)

%.o:: %.c
	@echo "Compiling: $<"
	@$(CC) -c $(THUMB) $(ALL_CFLAGS) $< -o $@

%.o:: %.s
	@echo "Compiling: $<"
	@$(CC) -c $(THUMB) $(ALL_ASFLAGS) $< -o $@

.PHONY:clean all ECHO clean_deps inc_dirs src_files

clean:
	- @rm $(ALL_OBJ_FILE)
	- @rm $(subst .o,.lst,$(ALL_OBJ_FILE))

clean_deps:
	@rm $(subst .c,.d,$(ALL_SRC_FILE))

inc_dirs:
	@echo $(ALL_INC_PATH)

src_files: 
	@echo $(ALL_SRC_FILE)

-include ${patsubst %.c,%.d,$(ALL_SRC_FILE)}

%.d: %.c
	@$(CC) -MM $(DEFINES) -I. $(addprefix -I ,$(ALL_INC_PATH)) $< > $@.$$$$;	\
	sed 's,\($(notdir $*)\)\.o[ :]*,$(dir $*)\1.o $@ : ,g' < $@.$$$$ > $@;	\
	rm -f $@.$$$$

