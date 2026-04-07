# HTTP Server Design

## Overview

The unikernel includes a minimal HTTP server for serving LLM inference requests. Since we're in a unikernel environment, standard networking libraries are not available -- the server is built on top of a custom network stack with PCI enumeration and an Intel e1000 NIC driver.

## Implementation Approach

### Network Stack

1. **Ethernet Frame Handling** -- Raw packet processing via e1000 driver
2. **IP Protocol** -- Basic IPv4 implementation
3. **TCP Protocol** -- Connection-oriented transport
4. **HTTP/1.1 Subset** -- Only the methods needed for our API

### API Endpoints

| Endpoint | Method | Description |
|---|---|---|
| `/health` | GET | Health check |
| `/models/load` | POST | Load a model |
| `/models/unload` | POST | Unload current model |
| `/completion` | POST | Generate text completion |
| `/embedding` | POST | Generate embeddings |

## Key Components

1. **Network Interface** (`network.c`) -- PCI device discovery, e1000 driver, packet I/O
2. **HTTP Parser** (`http.c`) -- Parse incoming HTTP/1.1 requests
3. **Router** (`api.c`) -- Route requests to appropriate handlers
4. **JSON Handler** (`json.c`) -- Parse and generate JSON request/response bodies
5. **Connection Manager** -- Handle connections via the network loop

## Simplifications for Unikernel

- **Single Threaded** -- No complex threading; one request at a time
- **Limited Connections** -- Sequential request processing
- **No File System** -- Serve responses directly from memory
- **Static Configuration** -- All configuration baked in at build time

## Implementation Order

1. Basic network interface -- get packets in and out
2. HTTP request parser -- parse incoming requests
3. Simple response generator -- generate HTTP responses
4. API endpoint handlers -- implement each endpoint
5. Integration with llama.cpp -- connect to LLM engine
