#!/bin/bash
# Host-side unit tests for CLLM components
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
SRC_DIR="${PROJECT_ROOT}/src"
INCLUDE_DIR="${PROJECT_ROOT}/include"
BUILD_DIR="${PROJECT_ROOT}/build"

echo "Building host-side allocator test..."
mkdir -p "${BUILD_DIR}"

gcc \
    -I"${INCLUDE_DIR}" \
    -fno-builtin-malloc -fno-builtin-free -fno-builtin-realloc -fno-builtin-memcpy \
    "${SCRIPT_DIR}/test_allocator.c" \
    "${SRC_DIR}/allocator.c" \
    "${SRC_DIR}/string.c" \
    -o "${BUILD_DIR}/test_allocator"

echo "Running allocator tests..."
"${BUILD_DIR}/test_allocator"

echo "Host tests passed."
