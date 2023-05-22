JL_SDK_DIR = libswitch/libjl51xx
SCRIPT_DIR = $(src)/$(JL_SDK_DIR)/scripts
INC_DIR = $(src)/$(JL_SDK_DIR)/inc
SRC_DIR = $(JL_SDK_DIR)/src

$(if $(wildcard .config),, $(error No .config exists!))
$(if $(wildcard $(SCRIPT_DIR)/Makefile.conf),, $(error No Makefile.conf exists!))
UC = $(shell echo '$1' | tr '[:lower:]' '[:upper:]')

include .config
include $(SCRIPT_DIR)/Makefile.conf

ccflags-$(CONFIG_JL51XX) += -DMDC_MDIO_OPERATION
ccflags-$(CONFIG_JL51XX) += -I$(INC_DIR)/

ccflags-$(CONFIG_JL51XX) += $(CFLAGS)

CONFIG_MODULE_NAME = 51xx
51xx-$(CONFIG_JL51XX) += \
	$(SRC_DIR)/acl.o \
	$(SRC_DIR)/cpu.o \
	$(SRC_DIR)/fc.o \
	$(SRC_DIR)/jl_error.o \
	$(SRC_DIR)/jl_switch.o \
	$(SRC_DIR)/l2.o \
	$(SRC_DIR)/led.o \
	$(SRC_DIR)/lpd.o \
	$(SRC_DIR)/mib.o \
	$(SRC_DIR)/mirror.o \
	$(SRC_DIR)/port.o \
	$(SRC_DIR)/qos.o \
	$(SRC_DIR)/storm.o \
	$(SRC_DIR)/stp.o \
	$(SRC_DIR)/vlan.o \
	$(SRC_DIR)/driver/hal_smi_mdio.o \
	$(SRC_DIR)/driver/jl_reg_io.o \
	$(SRC_DIR)/jl51xx/jl51xx_drv_acl.o \
	$(SRC_DIR)/jl51xx/jl51xx_drv_cpu.o \
	$(SRC_DIR)/jl51xx/jl51xx_drv_fc.o \
	$(SRC_DIR)/jl51xx/jl51xx_drv_l2.o \
	$(SRC_DIR)/jl51xx/jl51xx_drv_led.o \
	$(SRC_DIR)/jl51xx/jl51xx_drv_lpd.o \
	$(SRC_DIR)/jl51xx/jl51xx_drv_mib.o \
	$(SRC_DIR)/jl51xx/jl51xx_drv_mirror.o \
	$(SRC_DIR)/jl51xx/jl51xx_drv_phy.o \
	$(SRC_DIR)/jl51xx/jl51xx_drv_port.o \
	$(SRC_DIR)/jl51xx/jl51xx_drv_qos.o \
	$(SRC_DIR)/jl51xx/jl51xx_drv_storm.o \
	$(SRC_DIR)/jl51xx/jl51xx_drv_stp.o \
	$(SRC_DIR)/jl51xx/jl51xx_drv_switch.o \
	$(SRC_DIR)/jl51xx/jl51xx_drv_vlan.o \
	$(SRC_DIR)/jl51xx/jl51xx_mac_uctrl.o \
	portable/demo_kernel_osdep.o \
	portable/demo_kernel_mdio.o \
	test/jl_test_proc.o

$(CONFIG_MODULE_NAME)-y := $(51xx-y)
obj-$(CONFIG_JL51XX) += $(CONFIG_MODULE_NAME).o

