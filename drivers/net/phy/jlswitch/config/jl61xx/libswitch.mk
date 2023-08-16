JL_SDK_DIR := $(src)/libswitch/libjl61xx
SCRIPT_DIR := $(JL_SDK_DIR)/scripts
INC_DIR := $(JL_SDK_DIR)/inc
DRV_DIRS :=
SRC_DIRS := $(JL_SDK_DIR)/src
SRC_DIRS += $(JL_SDK_DIR)/src/drv_common
DRIVER_DIR := $(JL_SDK_DIR)/src/driver

include .config
include $(SCRIPT_DIR)/Makefile.conf

SRC_DIRS += $(DRV_DIRS)
SRC_FILES := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
SRC_FILES += $(DRIVER_DIR)/smi/hal_jl61xx_smi.c
SRC_FILES += $(DRIVER_DIR)/smi/hal_smi_mdio.c
SRC_FILES += $(DRIVER_DIR)/smi/hal_smi_spi.c
SRC_FILES += $(DRIVER_DIR)/spi/hal_spi.c
SRC_FILES += $(DRIVER_DIR)/spi/hal_jl61xx_spi.c
SRC_FILES += $(DRIVER_DIR)/jl_reg_ida.c
SRC_FILES += $(DRIVER_DIR)/jl_reg_io.c

OBJ_FILES := $(patsubst $(src)%.c, %.o, $(SRC_FILES))

#$(info "========SRC_FILES=$(SRC_FILES)")

ccflags-$(CONFIG_JL61XX) += -I$(INC_DIR)/
ccflags-$(CONFIG_JL61XX) += $(CFLAGS)
ccflags-$(CONFIG_JL61XX) += -std=gnu99

CONFIG_MODULE_NAME = 61xx
61xx-$(CONFIG_JL61XX) += \
	$(OBJ_FILES) \
	portable/demo_kernel_osdep.o \
	portable/demo_kernel_mdio.o \
	portable/demo_kernel_spi.o \
	devswitch/jl_spi_dev.o	\
	devswitch/jl_switch_dev.o

$(CONFIG_MODULE_NAME)-y := $(61xx-y)
obj-$(CONFIG_JL61XX) += $(CONFIG_MODULE_NAME).o

