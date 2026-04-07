# Project Specification

The goal of this project is to accomplish the following in phases:

1. Have a working C unikernel in QEMU targeting x86
2. Compile llama.cpp into the unikernel to serve models over HTTP, with a JSON-based configuration provided for the unikernel build
3. Add GPU support to the unikernel, supporting all backends (starting with CUDA) as done by llama.cpp
4. Port optimizations from vLLM into llama.cpp to accelerate serving of transformer models

Each phase is important and needs to be done correctly. The project builds the right code abstractions with detailed comments throughout.
