#!/bin/bash
# Build and run the host-side llama.cpp API integration test.
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build/llama-host"

"${SCRIPT_DIR}/build_llama.sh"

echo "Building llama API test..."
g++ -std=c++17 \
    -I"${PROJECT_ROOT}/llama.cpp/include" \
    -I"${PROJECT_ROOT}/llama.cpp/ggml/include" \
    "${SCRIPT_DIR}/test_llama_api.cpp" \
    "${BUILD_DIR}/src/libllama.so" \
    "${BUILD_DIR}/ggml/src/libggml.so" \
    -Wl,-rpath,"${BUILD_DIR}/src" \
    -Wl,-rpath,"${BUILD_DIR}/ggml/src" \
    -o "${BUILD_DIR}/test_llama_api"

echo "Running llama API test..."
LD_LIBRARY_PATH="${BUILD_DIR}/src:${BUILD_DIR}/ggml/src" \
    "${BUILD_DIR}/test_llama_api" "${1:-}"

echo "llama.cpp API test passed."
