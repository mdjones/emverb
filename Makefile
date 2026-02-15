# Project Name
TARGET = emverb

# Library Locations — override with env vars or edit these paths
LIBDAISY_DIR ?= ../../libDaisy
DAISYSP_DIR  ?= ../../DaisySP

# Sources
CPP_SOURCES = emverb.cpp

# Optimisation (O2 is a good balance for DSP)
OPT = -O2

# Core location and toolchain
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

# ── Convenience targets ─────────────────────────────────────
.PHONY: flash-dfu
flash-dfu: $(BUILD_DIR)/$(TARGET).bin
	dfu-util -a 0 -s 0x08000000:leave -D $<
