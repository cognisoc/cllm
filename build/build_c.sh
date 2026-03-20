#!/bin/bash
# build_c.sh - Build script for C components

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

# Determine compiler flags based on build mode
if [[ "${BUILD_MODE}" == "debug" ]]; then
    CFLAGS="-g -O0 -Wall -Wextra -I${INCLUDE_DIR} -fno-stack-protector"
    print_info "Building in debug mode"
else
    CFLAGS="-O2 -Wall -Wextra -I${INCLUDE_DIR} -fno-stack-protector"
    print_info "Building in release mode"
fi

# Determine architecture-specific flags
if [[ "${TARGET_ARCH}" == "x86_64" ]]; then
    ARCH_FLAGS="-m64"
elif [[ "${TARGET_ARCH}" == "i386" ]] || [[ "${TARGET_ARCH}" == "x86" ]]; then
    ARCH_FLAGS="-m32"
else
    print_error "Unsupported architecture: ${TARGET_ARCH}"
    exit 1
fi

# Create build directory
mkdir -p "${BUILD_DIR}/c_objects"

# Find C source files
C_SOURCES=()
while IFS= read -r -d '' file; do
    C_SOURCES+=("$file")
done < <(find "${SRC_DIR}" -name "*.c" -print0 2>/dev/null || true)

# Find assembly source files
ASM_SOURCES=()
while IFS= read -r -d '' file; do
    ASM_SOURCES+=("$file")
done < <(find "${SRC_DIR}" -name "*.S" -print0 2>/dev/null || true)

# Check if we found any source files
if [[ ${#C_SOURCES[@]} -eq 0 && ${#ASM_SOURCES[@]} -eq 0 ]]; then
    print_warning "No C or assembly source files found in ${SRC_DIR}"
    exit 0
fi

print_info "Found ${#C_SOURCES[@]} C source files and ${#ASM_SOURCES[@]} assembly source files"

# Compile each C source file
for source_file in "${C_SOURCES[@]}"; do
    # Get relative path for object file name
    rel_path="${source_file#${SRC_DIR}/}"
    obj_name="${rel_path%.c}.o"
    obj_path="${BUILD_DIR}/c_objects/${obj_name}"
    
    # Create directory structure if needed
    obj_dir=$(dirname "${obj_path}")
    mkdir -p "${obj_dir}"
    
    # Compile the source file
    print_info "Compiling ${rel_path}..."
    gcc ${ARCH_FLAGS} ${CFLAGS} -c "${source_file}" -o "${obj_path}" || {
        print_error "Failed to compile ${source_file}"
        exit 1
    }
done

# Compile each assembly source file
for source_file in "${ASM_SOURCES[@]}"; do
    # Get relative path for object file name
    rel_path="${source_file#${SRC_DIR}/}"
    obj_name="${rel_path%.S}.o"
    obj_path="${BUILD_DIR}/c_objects/${obj_name}"
    
    # Create directory structure if needed
    obj_dir=$(dirname "${obj_path}")
    mkdir -p "${obj_dir}"
    
    # Compile the source file
    print_info "Compiling ${rel_path}..."
    gcc ${ARCH_FLAGS} ${CFLAGS} -c "${source_file}" -o "${obj_path}" || {
        print_error "Failed to compile ${source_file}"
        exit 1
    }
done

print_success "C components built successfully!"
print_info "C object files are in ${BUILD_DIR}/c_objects"