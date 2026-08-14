###############################################################################
 #
 # Copyright (C) 2024 Analog Devices, Inc.
 #
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at
 #
 #     http://www.apache.org/licenses/LICENSE-2.0
 #
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
 #
 ##############################################################################
# This file can be used to set build configuration
# variables.  These variables are defined in a file called
# "Makefile" that is located next to this one.

# For instructions on how to use this system, see
# https://analogdevicesinc.github.io/msdk/USERGUIDE/#build-system

# Flat build directory; `make clean` removes exactly this.
BUILD_DIR ?= $(CURDIR)/build

# Compile-time log verbosity: [NONE, ERROR, WARN, INFO, DEBUG] (make LOG_LEVEL=DEBUG).
LOG_LEVEL ?= INFO

LIB_FREERTOS = 1
# BOARD selects the SDK BSP. Both profiles use FTHR_RevA for now: debug = the
# eval board (native), release = custom production board reusing it as a base
# (pin deltas handled in board_init.h / drivers) until a dedicated BSP exists.
BOARD := FTHR_RevA

# --- Build profile: debug (default) vs release --------------------------------
# Configure build-specific compiler/linker flags and logging options.
ifneq ($(filter release,$(MAKECMDGOALS)),)
  # Release: non-debug build. Explicit 0 so combined goals (release flash.openocd)
  # don't leave DEBUG empty (the SDK would then add -DDEBUG).
  DEBUG := 0
else
  # Debug (default): eval board, symbols, logging and RTOS stats on.
  DEBUG := 1
  PROJ_CFLAGS  += -DRTOS_RUNTIME_STATS
  PROJ_CFLAGS  += -DCONSOLE_UART=2
  PROJ_CFLAGS  += -DEVALBOARD
  # Logging (logger.h only emits under DEBUG; LOG_LEVEL selects verbosity).
  PROJ_CFLAGS  += -DACTIVE_LOG_LEVEL=NAQILOG_LEVEL_$(LOG_LEVEL)
#   PROJ_CFLAGS  += -DLOG_FILE_LINE_ENABLED
  PROJ_CFLAGS  += -DLOG_COLOR_ENABLED
  # Linker: pull in newlib float printf so the logger can print floats.
  PROJ_LDFLAGS += -u _printf_float
endif

# Always-on defines (target/library features, present in every build).
PROJ_CFLAGS += -DCMSIS_NN
PROJ_CFLAGS += -DTF_LITE_DISABLE_X86_NEON
PROJ_CFLAGS += -DTF_LITE_STATIC_MEMORY

# Warning set
PROJ_CFLAGS += -Wextra -Wno-unused-parameter

# Disable double-promotion warning enabled by default in the SDK (gcc.mk)
PROJ_CFLAGS += -Wno-double-promotion

# Rebuild when the profile or a command-line flag changes (these aren't tracked
# in any file). DEBUG is resolved above, so this stamp differs debug vs release;
# on a change, touch this makefile - every object depends on $(PROJECTMK).
BUILD_CONFIG_SIG := DEBUG=$(DEBUG)|LOG=$(LOG_LEVEL)|OPT=$(MXC_OPTIMIZE_CFLAGS)
$(shell mkdir -p '$(BUILD_DIR)' 2>/dev/null; \
        if [ "`cat '$(BUILD_DIR)/.config-sig' 2>/dev/null`" != '$(BUILD_CONFIG_SIG)' ]; then \
          printf '%s' '$(BUILD_CONFIG_SIG)' > '$(BUILD_DIR)/.config-sig'; \
          touch '$(PROJECTMK)'; \
        fi)

# Propagate DEBUG to the librtos/periphdriver/board sub-makes (drops -DDEBUG/-g3
# from the SDK libraries too in release / DEBUG=0).
export DEBUG

SRCS += $(shell find firmware-library-processing -name '*.c' -o -name '*.cpp' -o -name '*.cc')
SRCS += $(shell find tensorflow-lite-microcontrollers -name '*.c' -o -name '*.cpp' -o -name '*.cc')

IPATH += $(shell find firmware-library-processing -type d)
IPATH += $(shell find tensorflow-lite-microcontrollers -type d)

# src/ and include/ are organized into per-subsystem subdirectories
# (exg, imu, com, processing, system) - pick up sources/headers from all of them.
VPATH += $(shell find src -type d)
IPATH += $(shell find include -type d)

# Can provide a value for the FREERTOS heap allocation scheme
# Default value is 4
# FREERTOS_HEAP_TYPE := 2
# export FREERTOS_HEAP_TYPE

ifeq ($(BOARD),Aud01_RevA)
$(error ERR_NOTSUPPORTED: This project is not supported for the Audio board)
endif

# ---- Formatting (clang-format) -----------------------------------------------
# Project-owned sources only (not the third-party library trees).
FORMAT_FILES := $(shell find src include -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \))

.PHONY: lint format
# lint  = CHECK only (read-only; fails if anything is mis-formatted). Same as CI.
lint:
	clang-format --dry-run --Werror $(FORMAT_FILES)

# format = APPLY in place (run locally to fix violations, then commit).
format:
	clang-format -i $(FORMAT_FILES)

$(BUILD_DIR)/%.o: %.cc $(PROJECTMK) | $(BUILD_DIR)
	@echo -  CXX    ${<}
	@$(CXX) $(CXXFLAGS) -o $@ $<