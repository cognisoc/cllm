# C Unikernel for LLM Serving - Architecture Overview

```
+-----------------------------------------------------+
|                    QEMU Environment                 |
+-----------------------------------------------------+
|                                                     |
|  +------------------+    +----------------------+   |
|  |   HTTP Server    |----|  API Endpoints       |   |
|  |                  |    |  - /health           |   |
|  | Request Parsing  |    |  - /models/load      |   |
|  | Response Gen     |    |  - /completion       |   |
|  +------------------+    +----------------------+   |
|            |                      |                 |
|            |            +----------------------+   |
|            |            |  llama.cpp Library   |   |
|            |            |  (To be integrated)  |   |
|            |            +----------------------+   |
|            |                      |                 |
|  +------------------+    +----------------------+   |
|  | Kernel Services  |----|  System Components   |   |
|  |                  |    |  - VGA Output        |   |
|  | Memory Mgmt      |    |  - (Future: Network) |   |
|  | I/O Handling     |    |  - (Future: Storage) |   |
|  +------------------+    +----------------------+   |
|                                                     |
|  +------------------+                                |
|  |   Bootloader     |                                |
|  | (Multiboot Spec) |                                |
|  +------------------+                                |
|                                                     |
+-----------------------------------------------------+
```

## Component Descriptions

### Bootloader
- Implements the multiboot specification
- Loads our kernel into memory
- Sets up initial execution environment

### Kernel Services
- **Memory Management**: Basic memory allocation and management
- **I/O Handling**: VGA text output for debugging and status
- **System Services**: Foundation for higher-level components

### HTTP Server
- **Request Parsing**: Parses incoming HTTP requests
- **Response Generation**: Creates properly formatted HTTP responses
- **Routing**: Directs requests to appropriate handlers

### API Endpoints
- **/health**: System health check endpoint
- **/models/load**: Model loading endpoint (placeholder)
- **/completion**: Text completion endpoint (placeholder)

### llama.cpp Library
- **Core Inference Engine**: The actual LLM inference engine
- **Model Loading**: Handles loading of GGUF format models
- **Text Generation**: Generates text completions from prompts

## Data Flow

1. **Boot Process**: Bootloader loads and transfers control to kernel
2. **Kernel Initialization**: Kernel services are initialized
3. **HTTP Server Start**: HTTP server begins listening for requests
4. **Request Handling**: 
   - Incoming HTTP requests are parsed
   - Requests are routed to appropriate API handlers
   - Handlers process requests and generate responses
5. **Response Generation**: HTTP responses are formatted and sent back

## Future Extensions

### Phase 4: JSON Configuration
```
Build System
     |
     | (JSON Config)
     ↓
Customized Unikernel
```

### Phase 5: GPU Support
```
+------------------+
|   CUDA Driver    |
+------------------+
         |
         ↓
+------------------+
|  GPU Memory Mgmt |
+------------------+
         |
         ↓
+------------------+
| llama.cpp + CUDA |
+------------------+
```

### Phase 6: vLLM Optimizations
```
+------------------+
|  PagedAttention  |
+------------------+
         |
         ↓
+------------------+
| Continuous Batch |
+------------------+
         |
         ↓
+------------------+
| Optimized llama.cpp |
+------------------+
```