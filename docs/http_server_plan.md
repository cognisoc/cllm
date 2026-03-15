# HTTP Server Implementation Plan

## Overview

We need to implement a minimal HTTP server within our unikernel to serve LLM inference requests. This server will:

1. Handle HTTP requests from clients
2. Parse REST API calls for LLM operations
3. Interface with our llama.cpp integration
4. Return JSON responses

## Implementation Approach

Since we're in a unikernel environment, we can't use standard networking libraries. We'll need to:

1. **Implement Basic TCP/IP Stack**: 
   - Handle Ethernet frames
   - Implement IP protocol
   - Implement TCP protocol
   - Handle HTTP parsing

2. **Simplified Approach**: 
   - Use existing network drivers if available
   - Focus on HTTP/1.1 subset needed for our API
   - Implement only essential HTTP methods (GET, POST)

3. **API Endpoints**:
   - `/health` - Health check endpoint
   - `/models/load` - Load a model
   - `/models/unload` - Unload current model
   - `/completion` - Generate text completion
   - `/embedding` - Generate embeddings

## Key Components

1. **Network Interface**: Handle incoming packets
2. **HTTP Parser**: Parse incoming HTTP requests
3. **Router**: Route requests to appropriate handlers
4. **JSON Handler**: Parse and generate JSON responses
5. **Connection Manager**: Handle multiple connections

## Simplifications for Unikernel

1. **Single Threaded**: No need for complex threading
2. **Limited Connections**: Handle one request at a time initially
3. **No File System**: Serve responses directly from memory
4. **Static Configuration**: Configure at build time

## Implementation Order

1. **Basic Network Interface**: Get packets in and out
2. **HTTP Request Parser**: Parse incoming requests
3. **Simple Response Generator**: Generate HTTP responses
4. **API Endpoint Handlers**: Implement each endpoint
5. **Integration with llama.cpp**: Connect to LLM engine