#!/bin/bash
# build_minimal.sh - Build script for minimal QEMU-testable kernel

set -e

# Configuration
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
SRC_DIR="${PROJECT_ROOT}/src"
INCLUDE_DIR="${PROJECT_ROOT}/include"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print functions
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_info "Building minimal kernel for QEMU testing..."

# Create build directory
mkdir -p "${BUILD_DIR}"

# Compile minimal bootloader
print_info "Compiling minimal bootloader..."
gcc -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I${INCLUDE_DIR} \
    -c "${SRC_DIR}/minimal_boot.S" -o "${BUILD_DIR}/minimal_boot.o" || {
    print_error "Failed to compile minimal bootloader"
    exit 1
}

# Compile minimal kernel
print_info "Compiling minimal kernel..."
gcc -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I${INCLUDE_DIR} \
    -c "${SRC_DIR}/minimal_kernel.c" -o "${BUILD_DIR}/minimal_kernel.o" || {
    print_error "Failed to compile minimal kernel"
    exit 1
}

# Link minimal kernel
print_info "Linking minimal kernel..."
ld -m elf_i386 -T "${BUILD_DIR}/minimal_linker.ld" \
    "${BUILD_DIR}/minimal_boot.o" "${BUILD_DIR}/minimal_kernel.o" \
    -o "${BUILD_DIR}/minimal_kernel.bin" || {
    print_error "Failed to link minimal kernel"
    exit 1
}

# Show final build information
KERNEL_BIN="${BUILD_DIR}/minimal_kernel.bin"
if [[ -f "${KERNEL_BIN}" ]]; then
    print_success "Minimal kernel build completed successfully!"
    print_info "Kernel binary: ${KERNEL_BIN}"
    print_info "Binary size: $(du -h "${KERNEL_BIN}" | cut -f1)"
    print_info "File type: $(file "${KERNEL_BIN}")"
else
    print_error "Build failed - kernel binary not found"
    exit 1
fi