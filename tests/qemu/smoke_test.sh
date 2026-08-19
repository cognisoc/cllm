#!/bin/bash
# QEMU smoke test for the CLLM kernel.
#
# Builds the kernel, boots it in QEMU with no network forwarding (to avoid
# host port conflicts), and checks that the expected boot milestones are
# printed on the serial console.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
LOG_FILE="${BUILD_DIR}/qemu_smoke.log"

QEMU="${QEMU:-qemu-system-i386}"
KERNEL="${BUILD_DIR}/kernel.bin"
TIMEOUT_SECONDS="${TIMEOUT_SECONDS:-10}"

MILESTONES=(
    "KERNEL: CLLM unikernel booting"
    "MEMORY: Memory management system initialized"
    "VGA: VGA hardware initialized"
    "TERMINAL: Terminal initialized"
    "network_init: Network initialization complete"
    "KERNEL: Self-tests passed"
    "KERNEL: Entering network loop"
)

echo "QEMU smoke test"
echo "==============="

echo "Building release kernel..."
make -C "${PROJECT_ROOT}" release

if [[ ! -f "${KERNEL}" ]]; then
    echo "ERROR: kernel binary not found at ${KERNEL}"
    exit 1
fi

echo "Booting kernel in QEMU (timeout ${TIMEOUT_SECONDS}s)..."
rm -f "${LOG_FILE}"

# Run QEMU with no host port forwarding to keep CI environments conflict-free.
timeout "${TIMEOUT_SECONDS}s" \
    "${QEMU}" \
    -kernel "${KERNEL}" \
    -serial file:"${LOG_FILE}" \
    -display none \
    -no-reboot \
    -m 128M \
    >/dev/null 2>&1 || true

if [[ ! -f "${LOG_FILE}" ]]; then
    echo "ERROR: QEMU did not produce a serial log"
    exit 1
fi

echo "Checking boot milestones..."
FAILED=0
for milestone in "${MILESTONES[@]}"; do
    if grep -q "${milestone}" "${LOG_FILE}"; then
        echo "  [OK]   ${milestone}"
    else
        echo "  [FAIL] ${milestone}"
        FAILED=1
    fi
done

if [[ "${FAILED}" -ne 0 ]]; then
    echo "==============="
    echo "Smoke test FAILED.  Last 30 lines of serial log:"
    tail -n 30 "${LOG_FILE}"
    exit 1
fi

echo "==============="
echo "Smoke test PASSED."
