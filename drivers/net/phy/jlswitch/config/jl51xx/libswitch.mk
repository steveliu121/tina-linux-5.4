JL_SDK_DIR := $(src)/libswitch/libjl51xx
SCRIPT_DIR := $(JL_SDK_DIR)/scripts
INC_DIR := $(JL_SDK_DIR)/inc
DRV_DIRS := $(JL_SDK_DIR)/src/jl51xx
SRC_DIRS := $(JL_SDK_DIR)/src
DRIVER_DIR := $(JL_SDK_DIR)/src/driver

UC = $(shell echo '$1' | tr '[:lower:]' '[:upper:]')
include .config
include $(SCRIPT_DIR)/Makefile.conf

SRC_DIRS += $(DRV_DIRS)
SRC_FILES := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
SRC_FILES += $(DRIVER_DIR)/hal_smi_mdio.c
SRC_FILES += $(DRIVER_DIR)/jl_reg_io.c

OBJ_FILES := $(patsubst $(src)%.c, %.o, $(SRC_FILES))

#$(info "========SRC_FILES=$(SRC_FILES)")

ccflags-$(CONFIG_JL51XX) += -I$(INC_DIR)/
ccflags-$(CONFIG_JL51XX) += $(CFLAGS)

CONFIG_MODULE_NAME = 51xx
51xx-$(CONFIG_JL51XX) += \
	$(OBJ_FILES) \
	portable/demo_kernel_osdep.o \
	portable/demo_kernel_mdio.o \
	devswitch/jl_switch_dev.o

$(CONFIG_MODULE_NAME)-y := $(51xx-y)
obj-$(CONFIG_JL51XX) += $(CONFIG_MODULE_NAME).o

