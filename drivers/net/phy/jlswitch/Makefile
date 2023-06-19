ifneq ($(CONFIG_JL51XX),)
CFG_DIR := $(src)/config/jl51xx
else
CFG_DIR := $(src)/config/jl61xx
endif
include $(CFG_DIR)/.config
include $(CFG_DIR)/libswitch.mk

clean-files += Module.symvers Module.markers modules modules.order

