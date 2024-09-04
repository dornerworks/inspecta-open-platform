#
# Copyright 2021, Breakaway Consulting Pty. Ltd.
# Copyright 2022, UNSW (ABN 57 195 873 179)
# Copyright 2024, DornerWorks
#
# SPDX-License-Identifier: BSD-2-Clause
#

ifeq ($(strip $(MICROKIT_SDK)),)
$(error MICROKIT_SDK must be specified)
endif

MICROKIT_SDK := $(realpath $(MICROKIT_SDK))

ifeq ($(strip $(BOARD)),)
$(error BOARD must be specified)
endif

# Specify that we use bash for all shell commands
SHELL=/bin/bash

# Default build directory, pass BUILD_DIR=<dir> to override
BUILD_DIR ?= build
# Default config is a debug build, pass CONFIG=<debug/release/benchmark> to override
CONFIG ?= debug

# @ivanv: Check for dependencies and make sure they are installed/in the path

# @ivanv: check that all dependencies exist

CC := clang
LD := ld.lld
MICROKIT_TOOL ?= $(MICROKIT_SDK)/bin/microkit

SYSTEM_DESCRIPTION := open-platform.system

IMAGE_FILE = $(BUILD_DIR)/loader.img
REPORT_FILE = $(BUILD_DIR)/report.txt

ELFS := vmm.elf

all: directories $(IMAGE_FILE)

directories:
	$(shell mkdir -p $(BUILD_DIR))

$(BUILD_DIR)/vmm.elf: vmm/$(BUILD_DIR)/vmm.elf
	cp vmm/$(BUILD_DIR)/vmm.elf $(BUILD_DIR)/vmm.elf

vmm/$(BUILD_DIR)/vmm.elf: .EXPORT_ALL_VARIABLES 
	make -C vmm
	
.EXPORT_ALL_VARIABLES:

$(IMAGE_FILE) $(REPORT_FILE): $(addprefix $(BUILD_DIR)/, $(ELFS)) $(SYSTEM_DESCRIPTION)
	$(MICROKIT_TOOL) $(SYSTEM_DESCRIPTION) --search-path $(BUILD_DIR) $(IMAGE_DIR) --board $(BOARD) --config $(CONFIG) -o $(IMAGE_FILE) -r $(REPORT_FILE)

clean:
	make -C vmm clean
	rm -rf $(BUILD_DIR)
