#!/bin/bash
# build32.sh - Build script for 32-bit unikernel

set -e

# Configuration
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
SRC_DIR="${PROJECT_ROOT}/src"
INCLUDE_DIR="${PROJECT_ROOT}/include"

# Default values
BUILD_MODE="release"
TARGET_ARCH="i386"

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

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --mode)
            BUILD_MODE="$2"
            shift 2
            ;;
        --arch)
            TARGET_ARCH="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  --mode MODE         Build mode (debug|release)"
            echo "  --arch ARCH         Target architecture"
            echo "  --help              Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

print_info "Building 32-bit kernel..."

# Build C components
print_info "Building C components..."
"${PROJECT_ROOT}/build/build_c.sh" --mode "${BUILD_MODE}" --arch "${TARGET_ARCH}"

# Build Zig components
print_info "Building Zig components..."
"${PROJECT_ROOT}/build/build_zig.sh" --mode "${BUILD_MODE}" --arch "${TARGET_ARCH}"

# Link everything together
print_info "Linking components..."
"${PROJECT_ROOT}/build/link_components.sh" --mode "${BUILD_MODE}" --arch "${TARGET_ARCH}"

# Rename the kernel binary to kernel32.bin
if [[ -f "${BUILD_DIR}/kernel.bin" ]]; then
    mv "${BUILD_DIR}/kernel.bin" "${BUILD_DIR}/kernel32.bin"
fi

# Show final build information
KERNEL_BIN="${BUILD_DIR}/kernel32.bin"
if [[ -f "${KERNEL_BIN}" ]]; then
    print_success "32-bit build completed successfully!"
    print_info "Kernel binary: ${KERNEL_BIN}"
    print_info "Binary size: $(du -h "${KERNEL_BIN}" | cut -f1)"
else
    print_error "Build failed - kernel binary not found"
    exit 1
fi