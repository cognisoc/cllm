#!/bin/bash
# build_model_embedding.sh - Build script for model embedding components

set -e

# Configuration
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
SRC_DIR="${PROJECT_ROOT}/src"
INCLUDE_DIR="${PROJECT_ROOT}/include"

# Default values
BUILD_MODE="release"
TARGET_ARCH="x86_64"

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
        --model)
            MODEL_FILE="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  --mode MODE         Build mode (debug|release)"
            echo "  --arch ARCH         Target architecture"
            echo "  --model FILE        Model file to embed (optional)"
            echo "  --help              Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Determine Zig build flags based on build mode
if [[ "${BUILD_MODE}" == "debug" ]]; then
    ZIG_FLAGS="-O Debug"
    print_info "Building in debug mode"
else
    ZIG_FLAGS="-O ReleaseFast"
    print_info "Building in release mode"
fi

# Determine architecture-specific flags
if [[ "${TARGET_ARCH}" == "x86_64" ]]; then
    ZIG_TARGET="x86_64-linux-none"
elif [[ "${TARGET_ARCH}" == "i386" ]] || [[ "${TARGET_ARCH}" == "x86" ]]; then
    ZIG_TARGET="i386-linux-none"
else
    print_warning "Using default target for ${TARGET_ARCH}"
    ZIG_TARGET=""
fi

# Add target flag if specified
if [[ -n "${ZIG_TARGET}" ]]; then
    ZIG_FLAGS="${ZIG_FLAGS} -target ${ZIG_TARGET}"
fi

# Create build directory
mkdir -p "${BUILD_DIR}/model_objects"

# Build model embedding components
print_info "Building model embedding components..."

# Compile model embedding Zig file
print_info "Compiling model_embedding.zig..."
zig build-obj "${SRC_DIR}/model_embedding.zig" ${ZIG_FLAGS} --name model_embedding || {
    print_error "Failed to compile model_embedding.zig"
    exit 1
}

# Move generated object file
if [[ -f "model_embedding.o" ]]; then
    mv "model_embedding.o" "${BUILD_DIR}/model_objects/"
fi

# Compile C model interface Zig file
print_info "Compiling c_model_interface.zig..."
zig build-obj "${SRC_DIR}/c_model_interface.zig" ${ZIG_FLAGS} --name c_model_interface || {
    print_error "Failed to compile c_model_interface.zig"
    exit 1
}

# Move generated object file
if [[ -f "c_model_interface.o" ]]; then
    mv "c_model_interface.o" "${BUILD_DIR}/model_objects/"
fi

# Compile test program
print_info "Compiling test_model_embedding.c..."
gcc -m64 -c "${SRC_DIR}/test_model_embedding.c" -o "${BUILD_DIR}/model_objects/test_model_embedding.o" -I "${INCLUDE_DIR}" || {
    print_error "Failed to compile test_model_embedding.c"
    exit 1
}

print_success "Model embedding components built successfully!"
print_info "Model object files are in ${BUILD_DIR}/model_objects"

# Link test program
print_info "Linking test program..."
gcc -m64 -fPIC \
    "${BUILD_DIR}/model_objects/test_model_embedding.o" \
    "${BUILD_DIR}/model_objects/model_embedding.o" \
    "${BUILD_DIR}/model_objects/c_model_interface.o" \
    -o "${BUILD_DIR}/test_model_embedding" || {
    print_error "Failed to link test program"
    exit 1
}

print_success "Test program linked successfully!"
print_info "Test program: ${BUILD_DIR}/test_model_embedding"