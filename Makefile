# Makefile - Convenience Makefile for the unikernel project

PROJECT_ROOT := $(realpath .)
BUILD_DIR := $(PROJECT_ROOT)/build
SRC_DIR := $(PROJECT_ROOT)/src
INCLUDE_DIR := $(PROJECT_ROOT)/include

# Default target
.DEFAULT_GOAL := all

# Phony targets
.PHONY: all clean debug release test help

# All targets
all: release

# Release build
release:
	@echo "Building release version..."
	@$(BUILD_DIR)/build.sh --release

# Debug build
debug:
	@echo "Building debug version..."
	@$(BUILD_DIR)/build.sh --debug

# Clean build
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean completed."

# Test build
test:
	@echo "Building test programs..."
	@$(BUILD_DIR)/build_test.sh

# Help
help:
	@echo "Available targets:"
	@echo "  all      - Build release version (default)"
	@echo "  release  - Build release version"
	@echo "  debug    - Build debug version"
	@echo "  clean    - Clean build directory"
	@echo "  test     - Build test programs"
	@echo "  help     - Show this help message"