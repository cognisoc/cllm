#!/bin/bash
# simple_test_build.sh - Simple build script for testing model embedding (Zig only)

set -e

# Configuration
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
SRC_DIR="${PROJECT_ROOT}/src"

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

print_info "Building simple model embedding test (Zig only)..."

# Create build directory
mkdir -p "${BUILD_DIR}/simple_test"

# Compile and run the main test
print_info "Testing model_embedding.zig directly..."
zig run "${SRC_DIR}/test_model_embedding.zig" || {
    print_error "Failed to run model embedding test"
    exit 1
}

print_success "Model embedding test completed successfully!"

# Compile the components to object files
print_info "Compiling components to object files..."

# Compile model embedding
zig build-obj "${SRC_DIR}/model_embedding.zig" -O Debug --name model_embedding
mv model_embedding.o "${BUILD_DIR}/simple_test/" 2>/dev/null || true

# Compile C interface
zig build-obj "${SRC_DIR}/c_model_interface.zig" -O Debug --name c_model_interface
mv c_model_interface.o "${BUILD_DIR}/simple_test/" 2>/dev/null || true

print_success "Compiled components to object files!"

# Show what we built
if [[ -d "${BUILD_DIR}/simple_test" ]]; then
    print_info "Built files:"
    ls -la "${BUILD_DIR}/simple_test/"
fi

print_info "Simple model embedding test build completed!"