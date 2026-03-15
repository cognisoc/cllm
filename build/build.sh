#!/bin/bash
# build.sh - Main build script for the unikernel project

set -e

# Configuration
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
SRC_DIR="${PROJECT_ROOT}/src"
INCLUDE_DIR="${PROJECT_ROOT}/include"

# Build modes
BUILD_MODE="release"  # Can be "debug" or "release"
TARGET_ARCH="x86_64"  # Target architecture

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
        --debug)
            BUILD_MODE="debug"
            shift
            ;;
        --release)
            BUILD_MODE="release"
            shift
            ;;
        --arch)
            TARGET_ARCH="$2"
            shift 2
            ;;
        --clean)
            CLEAN_BUILD=1
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  --debug             Build in debug mode"
            echo "  --release           Build in release mode (default)"
            echo "  --arch ARCH         Target architecture (default: x86_64)"
            echo "  --clean             Clean build directory before building"
            echo "  --help              Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Clean build directory if requested
if [[ -n "$CLEAN_BUILD" ]]; then
    print_info "Cleaning build directory..."
    rm -rf "${BUILD_DIR}"
fi

# Create build directory
mkdir -p "${BUILD_DIR}"

# Print build configuration
print_info "Build Configuration:"
print_info "  Project Root: ${PROJECT_ROOT}"
print_info "  Build Mode: ${BUILD_MODE}"
print_info "  Target Architecture: ${TARGET_ARCH}"
print_info "  Build Directory: ${BUILD_DIR}"

# Build C components
print_info "Building C components..."
"${PROJECT_ROOT}/build/build_c.sh" --mode "${BUILD_MODE}" --arch "${TARGET_ARCH}"

# Build Zig components
print_info "Building Zig components..."
"${PROJECT_ROOT}/build/build_zig.sh" --mode "${BUILD_MODE}" --arch "${TARGET_ARCH}"

# Link everything together
print_info "Linking components..."
"${PROJECT_ROOT}/build/link_components.sh" --mode "${BUILD_MODE}" --arch "${TARGET_ARCH}"

# Show final build information
KERNEL_BIN="${BUILD_DIR}/kernel.bin"
if [[ -f "${KERNEL_BIN}" ]]; then
    print_success "Build completed successfully!"
    print_info "Kernel binary: ${KERNEL_BIN}"
    print_info "Binary size: $(du -h "${KERNEL_BIN}" | cut -f1)"
else
    print_error "Build failed - kernel binary not found"
    exit 1
fi