#!/bin/bash
# create_test_model.sh - Script to create a simple test GGUF file

set -e

# Configuration
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TEST_MODEL_FILE="${PROJECT_ROOT}/test_model.gguf"

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

# Create a simple GGUF file
print_info "Creating test GGUF file..."

# Create a simple GGUF header
# GGUF magic number
echo -n "GGUF" > "${TEST_MODEL_FILE}"

# Version (3) - little endian
echo -ne "\x03\x00\x00\x00" >> "${TEST_MODEL_FILE}"

# Tensor count (0) - little endian
echo -ne "\x00\x00\x00\x00" >> "${TEST_MODEL_FILE}"

# KV count (0) - little endian
echo -ne "\x00\x00\x00\x00" >> "${TEST_MODEL_FILE}"

# Add some placeholder data
echo -ne "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F" >> "${TEST_MODEL_FILE}"

print_success "Created test GGUF file: ${TEST_MODEL_FILE}"

# Show file size
FILE_SIZE=$(stat -c%s "${TEST_MODEL_FILE}")
print_info "Test model file size: ${FILE_SIZE} bytes"