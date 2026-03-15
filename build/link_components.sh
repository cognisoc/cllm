#!/bin/bash
# link_components.sh - Link C and Zig components together

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

# Determine linker flags based on build mode and architecture
if [[ "${BUILD_MODE}" == "debug" ]]; then
    LINKER_FLAGS="-g"
    print_info "Linking in debug mode"
else
    LINKER_FLAGS=""  # No strip flag for release
    print_info "Linking in release mode"
fi

# Determine architecture-specific flags
if [[ "${TARGET_ARCH}" == "x86_64" ]]; then
    ARCH_FLAGS="-m elf_x86_64"
elif [[ "${TARGET_ARCH}" == "i386" ]] || [[ "${TARGET_ARCH}" == "x86" ]]; then
    ARCH_FLAGS="-m elf_i386"
else
    print_error "Unsupported architecture: ${TARGET_ARCH}"
    exit 1
fi

# Check if object directories exist
C_OBJECTS_DIR="${BUILD_DIR}/c_objects"
ZIG_OBJECTS_DIR="${BUILD_DIR}/zig_objects"

if [[ ! -d "${C_OBJECTS_DIR}" ]]; then
    print_warning "C object directory not found: ${C_OBJECTS_DIR}"
    C_OBJECTS=()
else
    # Find all C object files
    C_OBJECTS=()
    while IFS= read -r -d '' file; do
        C_OBJECTS+=("$file")
    done < <(find "${C_OBJECTS_DIR}" -name "*.o" -print0 2>/dev/null || true)
fi

if [[ ! -d "${ZIG_OBJECTS_DIR}" ]]; then
    print_warning "Zig object directory not found: ${ZIG_OBJECTS_DIR}"
    ZIG_OBJECTS=()
else
    # Find all Zig object files
    ZIG_OBJECTS=()
    while IFS= read -r -d '' file; do
        ZIG_OBJECTS+=("$file")
    done < <(find "${ZIG_OBJECTS_DIR}" -name "*.o" -print0 2>/dev/null || true)
fi

# Check if we have any object files to link
if [[ ${#C_OBJECTS[@]} -eq 0 && ${#ZIG_OBJECTS[@]} -eq 0 ]]; then
    print_error "No object files found to link"
    exit 1
fi

print_info "Found ${#C_OBJECTS[@]} C object files and ${#ZIG_OBJECTS[@]} Zig object files"

# Collect all object files
ALL_OBJECTS=("${C_OBJECTS[@]}" "${ZIG_OBJECTS[@]}")

# Determine linker script
LINKER_SCRIPT="${BUILD_DIR}/linker.ld"
if [[ ! -f "${LINKER_SCRIPT}" ]]; then
    print_warning "Linker script not found: ${LINKER_SCRIPT}"
    print_info "Creating a basic linker script..."
    
    cat > "${LINKER_SCRIPT}" << 'EOF'
/* Basic linker script for unikernel */
ENTRY(_start)

SECTIONS
{
    . = 1M;  /* Load kernel at 1MB */

    .text : {
        *(.multiboot)
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}
EOF
fi

# Link all object files together
KERNEL_OUTPUT="${BUILD_DIR}/kernel.bin"
print_info "Linking kernel to ${KERNEL_OUTPUT}..."

ld ${ARCH_FLAGS} ${LINKER_FLAGS} \
    -T "${LINKER_SCRIPT}" \
    "${ALL_OBJECTS[@]}" \
    -o "${KERNEL_OUTPUT}" || {
    print_error "Failed to link kernel"
    exit 1
}

print_success "Components linked successfully!"
print_info "Kernel binary: ${KERNEL_OUTPUT}"
print_info "Binary size: $(du -h "${KERNEL_OUTPUT}" | cut -f1)"