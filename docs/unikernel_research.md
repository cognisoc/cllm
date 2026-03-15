# Unikernel Research

## Overview of C-based Unikernel Implementations

### Rumprun
Rumprun is a significant C-based unikernel implementation designed to run existing POSIX applications with minimal modifications:

- Built on the rump kernel concept
- Supports unikernel deployment targeting Xen and KVM/QEMU hypervisors
- Compatible with x86_64 architecture
- Can run unmodified NetBSD kernel drivers
- Provides a POSIX-like interface to ease application porting

### CloudABI
CloudABI is a security-oriented POSIX-like runtime environment that can be used in unikernel contexts:

- Provides capability-based security model
- Designed for running applications in isolated environments
- Offers C library implementations that work in unikernel settings
- Supports x86_64 architecture and works with QEMU for development

### IncludeOS
While primarily C++-based, IncludeOS has C compatibility:

- Runs on x86_64 architecture
- Compatible with QEMU/KVM
- Provides a minimal POSIX-like interface
- Built specifically for cloud deployment

## Recommendation for This Project

For our LLM serving unikernel, we should consider building on existing unikernel frameworks like Rumprun rather than implementing everything from scratch. This approach would allow us to:

1. Leverage existing kernel services (networking, memory management)
2. Focus on our core value proposition (LLM serving optimizations)
3. Reduce development time and potential bugs
4. Benefit from existing community support and documentation