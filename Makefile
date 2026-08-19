# Makefile - Convenience Makefile for the unikernel project

PROJECT_ROOT := $(realpath .)
BUILD_DIR := $(PROJECT_ROOT)/build
SRC_DIR := $(PROJECT_ROOT)/src
INCLUDE_DIR := $(PROJECT_ROOT)/include

# QEMU configuration
QEMU ?= qemu-system-i386
QEMU_FLAGS ?= -kernel $(BUILD_DIR)/kernel.bin -serial stdio -display none -no-reboot -m 128M -netdev user,id=net0,hostfwd=tcp::8080-:8080 -device e1000,netdev=net0

# Default target
.DEFAULT_GOAL := all

# Phony targets
.PHONY: all clean debug release run run-vga run-debug test test-llama smoke-test help

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

# Clean build artifacts (preserves build scripts)
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)/c_objects $(BUILD_DIR)/zig_objects $(BUILD_DIR)/zig-cache $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/kernel32.bin $(BUILD_DIR)/minimal_kernel.bin $(BUILD_DIR)/test_*
	@echo "Clean completed."

# Boot kernel in QEMU (serial output on terminal)
run: release
	@echo "Booting kernel in QEMU (serial on stdio)..."
	$(QEMU) $(QEMU_FLAGS)

# Boot kernel in QEMU with VGA display
run-vga: release
	@echo "Booting kernel in QEMU (VGA display)..."
	$(QEMU) -kernel $(BUILD_DIR)/kernel.bin -serial stdio -m 128M -netdev user,id=net0,hostfwd=tcp::8080-:8080 -device e1000,netdev=net0

# Boot kernel in QEMU paused for GDB
run-debug: debug
	@echo "Booting kernel in QEMU (waiting for GDB on :1234)..."
	$(QEMU) $(QEMU_FLAGS) -s -S

# Run host-side unit tests
test:
	@./tests/host/run_tests.sh

# Build and run llama.cpp host API integration test
test-llama:
	@./tests/llama/run_llama_test.sh

# Run QEMU smoke test (builds release kernel and checks boot milestones)
smoke-test:
	@./tests/qemu/smoke_test.sh

# Help
help:
	@echo "Available targets:"
	@echo "  all        - Build release version (default)"
	@echo "  release    - Build release version"
	@echo "  debug      - Build debug version"
	@echo "  clean      - Clean build artifacts"
	@echo "  run        - Build and boot in QEMU (serial output)"
	@echo "  run-vga    - Build and boot in QEMU (VGA window)"
	@echo "  run-debug  - Build debug and boot paused for GDB on :1234"
	@echo "  test       - Run host-side unit tests"
	@echo "  test-llama - Build and run llama.cpp host API test"
	@echo "  smoke-test - Build release kernel and run QEMU boot milestones"
	@echo "  help       - Show this help message"
