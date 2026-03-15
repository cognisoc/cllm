# Rumprun Setup Guide

## Installation Steps

1. **Clone the Rumprun repository**
```bash
git clone https://github.com/rumpkernel/rumprun.git
cd rumprun
```

2. **Install dependencies**
You'll need standard build tools plus QEMU:
```bash
# On Ubuntu/Debian
sudo apt-get install build-essential git qemu-system-x86

# On CentOS/RHEL/Fedora
sudo yum install gcc make git qemu-system-x86

# On macOS with Homebrew
brew install qemu
```

3. **Build Rumprun**
```bash
# Initialize submodules
git submodule update --init

# Build the toolchain (this will take a while)
./build-rr.sh hw -- -j$(nproc)
```

The build process will create the Rumprun toolchain and libraries in the `rumprun` directory.

4. **Set up environment variables**
```bash
export PATH="$(pwd)/rumprun/bin:${PATH}"
```

## Simple C Unikernel Example

Let's create a simple "Hello World" unikernel:

1. **Create a simple C program** (`hello.c`):
```c
#include <stdio.h>

int main() {
    printf("Hello from Rumprun unikernel!\n");
    return 0;
}
```

2. **Compile for Rumprun**:
```bash
# Compile with rumprun compiler
x86_64-rumprun-netbsd-gcc -o hello hello.c
```

3. **Create an ISO image**:
```bash
rumprun-bake hw_generic hello.iso hello
```

4. **Run in QEMU**:
```bash
rumprun qemu -i hello.iso
```

This will start QEMU and run your unikernel, which should output "Hello from Rumprun unikernel!" and then exit.

## Key Points

- Rumprun works with existing POSIX applications, so standard C programs can often run without modification
- The `rumprun-bake` command creates a bootable image for your target platform
- The `rumprun` command handles launching QEMU with appropriate parameters
- Rumprun supports multiple platforms including hw (bare metal/virtio) and Xen, but we're targeting QEMU with the hw platform