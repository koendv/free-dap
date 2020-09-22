###############################################################################
# free-dap

ifeq ($(MODULE_FREEDAP_ENABLED),1)
FREEDAP_DIR := $(USERMOD_DIR)
INC += \
  -I$(FREEDAP_DIR)/platform/$(MCU_ARCH)/include \
  -I$(FREEDAP_DIR)/include

SRC_MOD += $(addprefix $(FREEDAP_DIR)/,\
  mod_dap.c \
  platform/$(MCU_ARCH)/freedap.c \
  dap.c \
	)

endif
