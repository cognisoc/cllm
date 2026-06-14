#!/bin/bash
# Build the llama.cpp CPU backend for host-side API validation.
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
LLAMA_DIR="${PROJECT_ROOT}/llama.cpp"
BUILD_DIR="${PROJECT_ROOT}/build/llama-host"

echo "Building llama.cpp CPU backend (host) ..."
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

cmake "${LLAMA_DIR}" \
    -DLLAMA_BUILD_TESTS=OFF \
    -DLLAMA_BUILD_EXAMPLES=OFF \
    -DLLAMA_BUILD_SERVER=OFF \
    -DGGML_CUDA=OFF \
    -DGGML_METAL=OFF \
    -DGGML_VULKAN=OFF \
    -DGGML_OPENCL=OFF \
    -DGGML_SYCL=OFF \
    -DCMAKE_BUILD_TYPE=Release

make -j"$(nproc)" llama ggml

echo "llama.cpp host library built in ${BUILD_DIR}."
