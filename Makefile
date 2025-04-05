# Directory
ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

# Determine OS and Arch
ifeq ($(OS),Windows_NT)
  _OS := windows
  ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
    _ARCH := x86_64
  else
    ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
      _ARCH := x86_64
    endif
    ifeq ($(PROCESSOR_ARCHITECTURE),EM64T)
      _ARCH := x86_64
    endif
    ifeq ($(PROCESSOR_ARCHITECTURE),x86)
      _ARCH := x86
    endif
    ifeq ($(PROCESSOR_ARCHITECTURE),ARM64)
      _ARCH := arm64
    endif
  endif
else
  UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S),Linux)
    _OS := linux
  endif
  ifeq ($(UNAME_S),Darwin)
    _OS := darwin
  endif

  _PROC_ARCH := $(shell uname -m)

  ifeq ($(_PROC_ARCH),arm)
    _ARCH := arm
  endif
  ifeq ($(_PROC_ARCH),aarch64)
    _ARCH := arm64
  endif
  ifeq ($(_PROC_ARCH),aarch64_be)
    _ARCH := arm64
  endif
  ifeq ($(_PROC_ARCH),armv8b)
    _ARCH := arm64
  endif
  ifeq ($(_PROC_ARCH),armv8l)
    _ARCH := arm64
  endif
  ifeq ($(_PROC_ARCH),i386)
    _ARCH := x86
  endif
  ifeq ($(_PROC_ARCH),i686)
    _ARCH := x86
  endif
  ifeq ($(_PROC_ARCH),x86_64)
    _ARCH := x86_64
  endif
endif

# Filtered benchmarks
FILTERED_BMS :=template

# Compilation Configuratoin
CC:=gcc
IFLAGS:=-lpthread -lm
ifeq ($(_ARCH),x86_64)
  CFLAGS:=-mavx -mavx2 -g -O3
else ifeq ($(_ARCH),x86)
  CFLAGS:=-mavx -g -O3
else ifeq ($(_ARCH),arm64)
  CFLAGS:=-mfpu=neon -g -O3
endif

# File and directory names
BUILD_DIR := $(ROOT_DIR)/build
SRC_DIR := $(ROOT_DIR)/src

# Get all possible benchmarks
BENCHMARKS_ := $(notdir $(patsubst %/,%,$(dir $(abspath $(shell find $(SRC_DIR)/ -mindepth 2 -maxdepth 2 -name "Makefile.mk")))))
BENCHMARKS := $(filter-out $(FILTERED_BMS),$(BENCHMARKS_))

$(info Identified benchmarks are "$(BENCHMARKS)")

# Output
BINS_BM  := $(addprefix $(BUILD_DIR)/,$(BENCHMARKS))
CLEAN_BM := $(addprefix clean_,$(BENCHMARKS))

# Default
all: $(BINS_BM)

# Build directory
$(BUILD_DIR):
	mkdir -p $@

# Clean
clean: $(CLEAN_BM)
	rm -rf $(BUILD_DIR)

# Template
include template.mk

# All benchmarks/applications
-include $(SRC_DIR)/Makefile.mk

.PHONY: clean all