# Zig-Based HTTP API Server Design

## Overview

This document outlines the design for implementing a Zig-based HTTP API server that replicates the functionality of llama.cpp's server while leveraging Zig's safety features and concurrency model.

## Architecture

```
+-----------------------------------------------------+
|                   QEMU Environment                  |
+-----------------------------------------------------+
|                                                     |
|  +------------------+    +----------------------+   |
|  |   Zig HTTP API   |----|  Zig LLM Interface   |   |
|  | (Multi-threaded) |    | (Thread-safe)        |   |
|  +------------------+    +----------------------+   |
|            |                      |                 |
|            |            +----------------------+   |
|            |            |  llama.cpp Library   |   |
|            |            |  (Baked models)      |   |
|            |            +----------------------+   |
|            |                      |                 |
|  +------------------+    +----------------------+   |
|  | Kernel Services  |----|  System Components   |   |
|  | (C unikernel)    |    |  - Memory Mgmt       |   |
|  |                  |    |  - I/O Handling      |   |
|  +------------------+    +----------------------+   |
|                                                     |
|  +------------------+                                |
|  |   Bootloader     |                                |
|  | (Multiboot Spec) |                                |
|  +------------------+                                |
|                                                     |
+-----------------------------------------------------+
```

## Zig HTTP API Server Implementation

### 1. Core Server Structure

```zig
// http_server.zig
const std = @import("std");
const Allocator = std.mem.Allocator;

pub const HttpServer = struct {
    allocator: Allocator,
    port: u16,
    host: []const u8,
    thread_pool: ThreadPool,
    request_queue: RequestQueue,
    model_interface: ModelInterface,
    
    pub fn init(allocator: Allocator, host: []const u8, port: u16) !HttpServer {
        return HttpServer{
            .allocator = allocator,
            .host = try allocator.dupe(u8, host),
            .port = port,
            .thread_pool = try ThreadPool.init(allocator),
            .request_queue = try RequestQueue.init(allocator),
            .model_interface = try ModelInterface.init(allocator),
        };
    }
    
    pub fn deinit(self: *HttpServer) void {
        self.thread_pool.deinit();
        self.request_queue.deinit();
        self.model_interface.deinit();
        self.allocator.free(self.host);
    }
    
    pub fn start(self: *HttpServer) !void {
        // Start HTTP server loop
        // Handle incoming connections
        // Dispatch requests to thread pool
    }
};
```

### 2. Multi-threading with Thread Pool

```zig
// thread_pool.zig
const std = @import("std");
const Allocator = std.mem.Allocator;

pub const ThreadPool = struct {
    allocator: Allocator,
    threads: std.ArrayList(Thread),
    task_queue: TaskQueue,
    mutex: std.Thread.Mutex,
    cond: std.Thread.Condition,
    should_stop: bool,
    
    const Thread = struct {
        thread: std.Thread,
        id: usize,
    };
    
    pub fn init(allocator: Allocator) !ThreadPool {
        return ThreadPool{
            .allocator = allocator,
            .threads = std.ArrayList(Thread).init(allocator),
            .task_queue = try TaskQueue.init(allocator),
            .mutex = std.Thread.Mutex{},
            .cond = std.Thread.Condition{},
            .should_stop = false,
        };
    }
    
    pub fn deinit(self: *ThreadPool) void {
        self.should_stop = true;
        self.cond.broadcast();
        
        for (self.threads.items) |*thread| {
            thread.thread.join();
        }
        
        self.threads.deinit();
        self.task_queue.deinit();
    }
    
    pub fn start(self: *ThreadPool, num_threads: usize) !void {
        for (0..num_threads) |i| {
            const thread = try std.Thread.spawn(.{}, workerThread, .{ self, i });
            try self.threads.append(Thread{
                .thread = thread,
                .id = i,
            });
        }
    }
    
    fn workerThread(self: *ThreadPool, thread_id: usize) void {
        _ = thread_id;
        while (!self.should_stop) {
            const task = self.task_queue.dequeue() catch |err| {
                std.log.err("Failed to dequeue task: {}", .{err});
                continue;
            };
            
            if (task) |t| {
                t.execute() catch |err| {
                    std.log.err("Task execution failed: {}", .{err});
                };
                t.deinit();
            }
        }
    }
};
```

### 3. Request Handling

```zig
// request_handler.zig
const std = @import("std");
const Allocator = std.mem.Allocator;

pub const RequestHandler = struct {
    allocator: Allocator,
    
    pub fn handleCompletion(self: *RequestHandler, request: HttpRequest) !HttpResponse {
        // Parse completion request
        const params = try parseCompletionParams(self.allocator, request.body);
        defer params.deinit();
        
        // Queue request for processing
        const result = try self.model_interface.generateCompletion(params);
        
        // Format response
        return formatCompletionResponse(self.allocator, result);
    }
    
    pub fn handleChatCompletion(self: *RequestHandler, request: HttpRequest) !HttpResponse {
        // Parse chat completion request
        const params = try parseChatCompletionParams(self.allocator, request.body);
        defer params.deinit();
        
        // Queue request for processing
        const result = try self.model_interface.generateChatCompletion(params);
        
        // Format response
        return formatChatCompletionResponse(self.allocator, result);
    }
    
    pub fn handleEmbedding(self: *RequestHandler, request: HttpRequest) !HttpResponse {
        // Parse embedding request
        const params = try parseEmbeddingParams(self.allocator, request.body);
        defer params.deinit();
        
        // Generate embeddings
        const result = try self.model_interface.generateEmbedding(params);
        
        // Format response
        return formatEmbeddingResponse(self.allocator, result);
    }
    
    pub fn handleHealth(self: *RequestHandler, request: HttpRequest) !HttpResponse {
        _ = request;
        // Check model status
        const is_ready = self.model_interface.isModelReady();
        
        if (is_ready) {
            return HttpResponse{
                .status = 200,
                .content_type = "application/json",
                .body = try std.fmt.allocPrint(self.allocator, "{{\"status\": \"ok\"}}", .{}),
            };
        } else {
            return HttpResponse{
                .status = 503,
                .content_type = "application/json",
                .body = try std.fmt.allocPrint(self.allocator, 
                    "{{\"error\": {{\"code\": 503, \"message\": \"Loading model\", \"type\": \"unavailable_error\"}}}}", .{}),
            };
        }
    }
};
```

### 4. Streaming Responses

```zig
// streaming_response.zig
const std = @import("std");
const Allocator = std.mem.Allocator;

pub const StreamingResponse = struct {
    allocator: Allocator,
    writer: std.net.StreamServer.Connection.Writer,
    buffer: []u8,
    buffer_pos: usize,
    
    pub fn init(allocator: Allocator, writer: std.net.StreamServer.Connection.Writer) !StreamingResponse {
        return StreamingResponse{
            .allocator = allocator,
            .writer = writer,
            .buffer = try allocator.alloc(u8, 4096),
            .buffer_pos = 0,
        };
    }
    
    pub fn deinit(self: *StreamingResponse) void {
        self.allocator.free(self.buffer);
    }
    
    pub fn writeEvent(self: *StreamingResponse, data: []const u8) !void {
        // Format as Server-Sent Event
        try self.writer.print("data: {s}\n\n", .{data});
        try self.writer.flush();
    }
    
    pub fn writeCompletionToken(self: *StreamingResponse, token: []const u8) !void {
        // Format token as JSON for streaming
        const json_token = try std.fmt.allocPrint(self.allocator, "{{\"content\": \"{s}\"}}", .{token});
        defer self.allocator.free(json_token);
        
        try self.writeEvent(json_token);
    }
};
```

### 5. Model Interface (Thread-Safe)

```zig
// model_interface.zig
const std = @import("std");
const Allocator = std.mem.Allocator;

pub const ModelInterface = struct {
    allocator: Allocator,
    model_mutex: std.Thread.Mutex,
    model: ?*c_llama_model,
    context: ?*c_llama_context,
    
    pub fn init(allocator: Allocator) !ModelInterface {
        return ModelInterface{
            .allocator = allocator,
            .model_mutex = std.Thread.Mutex{},
            .model = null,
            .context = null,
        };
    }
    
    pub fn deinit(self: *ModelInterface) void {
        self.model_mutex.lock();
        defer self.model_mutex.unlock();
        
        if (self.context) |ctx| {
            c_llama_free(ctx);
            self.context = null;
        }
        
        if (self.model) |model| {
            c_llama_free_model(model);
            self.model = null;
        }
    }
    
    pub fn generateCompletion(self: *ModelInterface, params: CompletionParams) !CompletionResult {
        self.model_mutex.lock();
        defer self.model_mutex.unlock();
        
        if (self.model == null or self.context == null) {
            return error.ModelNotLoaded;
        }
        
        // Perform completion generation
        // This would call into the C llama.cpp API
        return generateCompletionWithModel(self.model.?, self.context.?, params);
    }
    
    pub fn isModelReady(self: *ModelInterface) bool {
        self.model_mutex.lock();
        defer self.model_mutex.unlock();
        
        return self.model != null and self.context != null;
    }
};
```

## Key Features Implementation

### 1. Multi-threading by Default
- Use Zig's async/await for non-blocking operations
- Implement thread pool for request processing
- Use channels for inter-task communication

### 2. Streaming Responses
- Implement Server-Sent Events for real-time output
- Support both chunked and token-level streaming
- Handle backpressure in streaming scenarios

### 3. Request Queuing
- Implement request queue for handling concurrent requests
- Support priority-based request processing
- Handle request timeouts and cancellations

### 4. Memory Safety
- Use Zig's allocator system for memory management
- Implement proper error handling with explicit error returns
- Use defer statements for resource cleanup

### 5. Performance Optimizations
- Pre-allocate buffers for common operations
- Use memory pools for request/response handling
- Implement zero-copy operations where possible

## Integration with C Unikernel

### 1. C Bindings
```zig
// c_bindings.zig
const std = @import("std");

// Import C llama.cpp functions
extern "c" fn llama_model_default_params() c_llama_model_params;
extern "c" fn llama_load_model_from_file(path: [*c]const u8, params: c_llama_model_params) ?*c_llama_model;
extern "c" fn llama_new_context_with_model(model: ?*c_llama_model, params: c_llama_context_params) ?*c_llama_context;
extern "c" fn llama_free(ctx: ?*c_llama_context) void;
extern "c" fn llama_free_model(model: ?*c_llama_model) void;

// Wrapper functions with Zig error handling
pub fn loadModel(path: []const u8) !*c_llama_model {
    const params = llama_model_default_params();
    const model = llama_load_model_from_file(path.ptr, params);
    
    if (model == null) {
        return error.ModelLoadFailed;
    }
    
    return model;
}
```

### 2. Baked Model Integration
- Embed model data directly in the executable
- Use memory mapping for efficient access
- Implement initialization routines for baked models

## Build System Integration

### 1. Model Baking Process
```bash
# Build script for baking models
#!/bin/bash
# Convert model to GGUF format
# Embed model data in executable
# Compile with embedded model data
```

### 2. Compilation
- Compile Zig components with appropriate flags
- Link with C llama.cpp library
- Optimize for target deployment

## Testing Strategy

### 1. Unit Tests
- Test individual components in isolation
- Verify error handling paths
- Check memory safety

### 2. Integration Tests
- Test end-to-end request processing
- Verify API compatibility with llama.cpp
- Check performance characteristics

### 3. Stress Tests
- Test concurrent request handling
- Verify resource usage under load
- Check for race conditions

## Performance Monitoring

### 1. Metrics Collection
- Track tokens per second
- Monitor memory usage
- Measure request latency

### 2. Health Monitoring
- Track server uptime
- Monitor model loading status
- Report hardware utilization

This design provides a solid foundation for implementing a Zig-based HTTP API server that replicates llama.cpp's functionality while leveraging Zig's safety features and concurrency model.