#!/bin/bash
# Build the llama.cpp CPU backend in a form that can be linked into the unikernel.
# This is an experimental/development build; full bare-metal C++ linking requires
# additional OS stubs and is not yet wired into the main kernel link step.
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
LLAMA_DIR="${PROJECT_ROOT}/llama.cpp"
BUILD_DIR="${PROJECT_ROOT}/build/llama-kernel"

echo "Configuring llama.cpp for kernel-compatible static library (experimental)..."
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

cmake "${LLAMA_DIR}" \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_C_FLAGS="-m32 -ffreestanding -fno-stack-protector -fno-exceptions -fno-rtti" \
    -DCMAKE_CXX_FLAGS="-m32 -ffreestanding -fno-stack-protector -fno-exceptions -fno-rtti" \
    -DLLAMA_BUILD_TESTS=OFF \
    -DLLAMA_BUILD_EXAMPLES=OFF \
    -DLLAMA_BUILD_SERVER=OFF \
    -DGGML_CUDA=OFF \
    -DGGML_METAL=OFF \
    -DGGML_VULKAN=OFF \
    -DGGML_OPENCL=OFF \
    -DGGML_SYCL=OFF \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_BUILD_TYPE=Release

# Build only the core static libraries; linking them into the kernel is the next step.
make -j"$(nproc)" llama_static ggml_static 2>/dev/null || make -j"$(nproc)" llama ggml

echo "Kernel-compatible static build attempted in ${BUILD_DIR}."
echo "Note: additional OS stub work is required to link these into the unikernel binary."
