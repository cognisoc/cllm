#!/bin/bash
# build_zig.sh - Build script for Zig components

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

# Determine Zig build flags based on build mode
if [[ "${BUILD_MODE}" == "debug" ]]; then
    ZIG_FLAGS="-O Debug --library c"
    print_info "Building in debug mode"
else
    ZIG_FLAGS="-O ReleaseFast --library c"
    print_info "Building in release mode"
fi

# Determine architecture-specific flags
if [[ "${TARGET_ARCH}" == "x86_64" ]]; then
    ZIG_TARGET="x86_64-linux-none"
elif [[ "${TARGET_ARCH}" == "i386" ]] || [[ "${TARGET_ARCH}" == "x86" ]]; then
    ZIG_TARGET="x86-linux-musl"
else
    print_warning "Using default target for ${TARGET_ARCH}"
    ZIG_TARGET=""
fi

# Add target flag if specified
if [[ -n "${ZIG_TARGET}" ]]; then
    ZIG_FLAGS="${ZIG_FLAGS} -target ${ZIG_TARGET}"
fi

# Create build directory
mkdir -p "${BUILD_DIR}/zig_objects"

# Find Zig source files
ZIG_SOURCES=()
while IFS= read -r -d '' file; do
    ZIG_SOURCES+=("$file")
done < <(find "${SRC_DIR}" -name "*.zig" -print0 2>/dev/null || true)

# Check if we found any Zig source files
if [[ ${#ZIG_SOURCES[@]} -eq 0 ]]; then
    print_warning "No Zig source files found in ${SRC_DIR}"
    exit 0
fi

print_info "Found ${#ZIG_SOURCES[@]} Zig source files"

# Compile each Zig source file to object file
for source_file in "${ZIG_SOURCES[@]}"; do
    # Get relative path for object file name
    rel_path="${source_file#${SRC_DIR}/}"
    obj_name="${rel_path%.zig}.o"
    obj_path="${BUILD_DIR}/zig_objects/${obj_name}"
    
    # Create directory structure if needed
    obj_dir=$(dirname "${obj_path}")
    mkdir -p "${obj_dir}"
    
    # Generate a simple name without path separators
    simple_name=$(basename "${obj_name%.o}")
    
    # Compile the source file to object file
    print_info "Compiling ${rel_path} to object file..."
    zig build-obj "${source_file}" ${ZIG_FLAGS} --name "${simple_name}" || {
        print_error "Failed to compile ${source_file}"
        exit 1
    }
    
    # Move the generated object file to the correct location
    generated_obj="${BUILD_DIR}/${simple_name}.o"
    if [[ -f "${generated_obj}" ]]; then
        mv "${generated_obj}" "${obj_path}"
    fi
done

print_success "Zig components built successfully!"
print_info "Zig object files are in ${BUILD_DIR}/zig_objects"