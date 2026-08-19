#!/usr/bin/env python3
"""
Integration smoke test for the CLLM HTTP server.

Runs the kernel in QEMU with user-mode networking, forwards host port 8080 to
the guest, and sends HTTP requests to verify the server responds.
"""

import socket
import subprocess
import sys
import time
import urllib.request

KERNEL = "build/kernel.bin"
BOOT_MARKER = "KERNEL: Entering network loop"


def find_free_port() -> int:
    """Return a free localhost TCP port."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(("127.0.0.1", 0))
        return int(s.getsockname()[1])


def qemu_command(host_port: int) -> list[str]:
    return [
        "qemu-system-i386",
        "-kernel", KERNEL,
        "-serial", "stdio",
        "-display", "none",
        "-no-reboot",
        "-m", "128M",
        "-netdev", f"user,id=net0,hostfwd=tcp::{host_port}-:8080",
        "-device", "e1000,netdev=net0",
    ]


def main() -> int:
    host_port = find_free_port()
    print(f"Starting QEMU with host port {host_port} forwarded to guest 8080...")
    proc = subprocess.Popen(
        qemu_command(host_port),
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="utf-8",
        errors="replace",
        bufsize=1,
    )

    # Wait for the kernel to finish booting.
    booted = False
    try:
        for line in proc.stdout:  # type: ignore[union-attr]
            print(line, end="")
            if BOOT_MARKER in line:
                booted = True
                break
    except Exception as exc:
        print(f"Error reading QEMU output: {exc}")
        proc.terminate()
        return 1

    if not booted:
        print("Kernel did not boot successfully")
        proc.terminate()
        return 1

    # Give the TCP stack a moment to settle.
    time.sleep(1)

    base = f"http://127.0.0.1:{host_port}"
    checks = [
        ("GET", f"{base}/health", None),
        ("POST", f"{base}/v1/completions", b'{"prompt":"hello","max_tokens":4}'),
        ("POST", f"{base}/v1/chat/completions", b'{"messages":[{"role":"user","content":"hi"}]}'),
        ("GET", f"{base}/v1/models", None),
    ]

    failures = 0
    for method, url, body in checks:
        req = urllib.request.Request(url, data=body, method=method)
        req.add_header("Content-Type", "application/json")
        try:
            with urllib.request.urlopen(req, timeout=5) as resp:
                data = resp.read().decode("utf-8", errors="replace")
                print(f"{method} {url} -> {resp.status}: {data[:120]}")
                if resp.status != 200:
                    failures += 1
        except urllib.error.HTTPError as exc:
            print(f"{method} {url} -> HTTP {exc.code}")
            failures += 1
        except Exception as exc:
            print(f"{method} {url} -> ERROR: {exc}")
            failures += 1

    proc.terminate()
    try:
        proc.wait(timeout=5)
    except subprocess.TimeoutExpired:
        proc.kill()

    if failures:
        print(f"FAILED: {failures} check(s) failed")
        return 1

    print("PASSED: HTTP smoke test")
    return 0


if __name__ == "__main__":
    sys.exit(main())
