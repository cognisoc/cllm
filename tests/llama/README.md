# llama.cpp Integration Status

This directory contains host-side build and test helpers for the upstream
llama.cpp submodule.

## What works today

- `run_llama_test.sh` builds llama.cpp `b4499` CPU backend on the host and runs
  a small C++ program that exercises the upstream C API.
- `src/llm.cpp` in the unikernel tree is a C++ bridge that compiles against the
  upstream headers. It provides `llm_generate()` and the standard `llm_*` C
  interface used by the HTTP API.

## What is still experimental

- Linking llama.cpp into the 32-bit bare-metal kernel binary requires:
  - C++ runtime stubs (new/delete, pure virtual, atexit)
  - Replacing llama.cpp's OS dependencies (file I/O, threading, dynamic backend loading)
  - A mechanism to provide the embedded GGUF model as a "file" or via direct data
  - 64-bit address space is strongly recommended for real models

The `build_llama_for_kernel.sh` script is a stub that documents the attempted
build flags. Running it in this environment will fail because 32-bit host
libraries are not installed; a real cross-compilation environment is needed.

## Next steps

1. Move the unikernel to a 64-bit target (Multiboot2 / UEFI) to accommodate real
   model sizes.
2. Provide a `FILE*` implementation backed by the embedded model byte array so
   `llama_model_load_from_file("/model.gguf", ...)` reads from memory.
3. Stub or disable llama.cpp's thread-pool and backend registration paths.
4. Link the resulting static libraries with the kernel objects and the C++
   runtime stubs in `src/llm.cpp`.
