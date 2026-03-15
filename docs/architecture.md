# Architecture Overview

This document provides a high-level overview of the C unikernel architecture for serving LLMs.

## Components

1. **Unikernel Core**: Minimal operating system services
2. **LLM Engine**: Integration of llama.cpp for model inference
3. **HTTP Server**: REST API for model interaction
4. **Configuration System**: JSON-based build and runtime configuration
5. **GPU Support**: CUDA and other backend integrations
6. **Optimizations**: Performance enhancements from vLLM

## Data Flow

1. Configuration is loaded at build time or startup
2. Models are loaded into memory
3. HTTP requests are processed by the server
4. LLM engine performs inference
5. Results are returned via HTTP responses

## Memory Management

The unikernel will implement a custom memory allocator optimized for LLM workloads.