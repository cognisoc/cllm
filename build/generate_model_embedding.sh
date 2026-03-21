#!/bin/bash
# generate_model_embedding.sh - Script to generate model embedding code from GGUF file

set -e

# Configuration
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
SRC_DIR="${PROJECT_ROOT}/src"

# Default values
MODEL_FILE=""
OUTPUT_FILE="${SRC_DIR}/embedded_model.zig"

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
        --model)
            MODEL_FILE="$2"
            shift 2
            ;;
        --output)
            OUTPUT_FILE="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  --model FILE        GGUF model file to embed"
            echo "  --output FILE       Output Zig file (default: ${OUTPUT_FILE})"
            echo "  --help              Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Check if model file is provided
if [[ -z "${MODEL_FILE}" ]]; then
    print_error "No model file provided"
    echo "Usage: $0 --model MODEL_FILE [--output OUTPUT_FILE]"
    exit 1
fi

# Check if model file exists
if [[ ! -f "${MODEL_FILE}" ]]; then
    print_error "Model file not found: ${MODEL_FILE}"
    exit 1
fi

# Get model file size
MODEL_SIZE=$(stat -c%s "${MODEL_FILE}")
print_info "Model file size: ${MODEL_SIZE} bytes"

# Generate Zig code to embed the model
print_info "Generating Zig code to embed model..."
echo "// embedded_model.zig - Embedded model data" > "${OUTPUT_FILE}"
echo "const embedded_model_data = &[_]u8{" >> "${OUTPUT_FILE}"

# Convert model file to Zig array
# We'll read the file in chunks to avoid memory issues with large files
CHUNK_SIZE=1024
OFFSET=0

while [[ ${OFFSET} -lt ${MODEL_SIZE} ]]; do
    # Calculate remaining bytes
    REMAINING=$((MODEL_SIZE - OFFSET))
    READ_SIZE=$((CHUNK_SIZE < REMAINING ? CHUNK_SIZE : REMAINING))
    
    # Read chunk and convert to hex
    dd if="${MODEL_FILE}" bs=1 skip=${OFFSET} count=${READ_SIZE} 2>/dev/null | \
    hexdump -v -e '/1 "0x%02x, "' >> "${OUTPUT_FILE}"
    
    OFFSET=$((OFFSET + READ_SIZE))
    
    # Add newline every few entries for readability
    if [[ $((OFFSET % (CHUNK_SIZE * 4))) -eq 0 ]]; then
        echo "" >> "${OUTPUT_FILE}"
    fi
done

echo "" >> "${OUTPUT_FILE}"
echo "};" >> "${OUTPUT_FILE}"
echo "" >> "${OUTPUT_FILE}"
echo "pub fn getEmbeddedModel() []const u8 {" >> "${OUTPUT_FILE}"
echo "    return embedded_model_data;" >> "${OUTPUT_FILE}"
echo "}" >> "${OUTPUT_FILE}"
echo "" >> "${OUTPUT_FILE}"
echo "pub fn getModelSize() usize {" >> "${OUTPUT_FILE}"
echo "    return embedded_model_data.len;" >> "${OUTPUT_FILE}"
echo "}" >> "${OUTPUT_FILE}"

print_success "Generated model embedding code to ${OUTPUT_FILE}"

# Show generated file size
GENERATED_SIZE=$(stat -c%s "${OUTPUT_FILE}")
print_info "Generated file size: ${GENERATED_SIZE} bytes"