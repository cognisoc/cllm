// memory_management.zig - Memory management across C and Zig boundaries
const std = @import("std");
const builtin = @import("builtin");

// Global allocator tracking
var global_allocator_bytes: usize = 0;
var global_allocator_mutex: std.Thread.Mutex = std.Thread.Mutex{};

// Memory pool implementation
pub const MemoryPool = struct {
    allocator: std.heap.FixedBufferAllocator,
    buffer: []u8,
    block_size: usize,
    
    pub fn init(block_size: usize, num_blocks: usize) !MemoryPool {
        const total_size = block_size * num_blocks;
        const buffer = try std.heap.c_allocator.alloc(u8, total_size);
        
        return MemoryPool{
            .allocator = std.heap.FixedBufferAllocator.init(buffer),
            .buffer = buffer,
            .block_size = block_size,
        };
    }
    
    pub fn deinit(self: *MemoryPool) void {
        std.heap.c_allocator.free(self.buffer);
    }
    
    pub fn alloc(self: *MemoryPool, size: usize) ![]u8 {
        if (size > self.block_size) {
            return error.OutOfMemory;
        }
        return self.allocator.allocator().alloc(u8, size);
    }
    
    pub fn free(self: *MemoryPool, ptr: []u8) void {
        // In a real implementation, we would need to track which blocks are free
        // For now, we'll just reset the allocator when needed
        _ = ptr;
        self.allocator.reset();
    }
};

// C-compatible memory management functions
export fn zig_malloc(size: usize) ?*anyopaque {
    global_allocator_mutex.lock();
    defer global_allocator_mutex.unlock();
    
    const ptr = std.heap.c_allocator.alloc(u8, size) catch return null;
    global_allocator_bytes += size;
    return @ptrCast(ptr.ptr);
}

export fn zig_free(ptr: ?*anyopaque) void {
    if (ptr) |p| {
        // In a real implementation, we would need to track the size of allocated blocks
        // For now, we'll just free the pointer and assume the caller knows the size
        _ = p;
        // std.heap.c_allocator.free(@as([*]u8, @ptrCast(p))[0..size]);
    }
}

export fn zig_realloc(ptr: ?*anyopaque, size: usize) ?*anyopaque {
    if (ptr == null) {
        return zig_malloc(size);
    }
    
    global_allocator_mutex.lock();
    defer global_allocator_mutex.unlock();
    
    // In a real implementation, we would need to track the size of the original block
    // For now, we'll just allocate new memory and copy data
    const new_ptr = std.heap.c_allocator.alloc(u8, size) catch return null;
    global_allocator_bytes += size;
    return @ptrCast(new_ptr.ptr);
}

export fn zig_get_memory_usage() usize {
    global_allocator_mutex.lock();
    defer global_allocator_mutex.unlock();
    return global_allocator_bytes;
}

// Memory pool functions
export fn zig_memory_pool_create(block_size: usize, num_blocks: usize) ?*MemoryPool {
    const allocator = std.heap.c_allocator;
    
    const pool = allocator.create(MemoryPool) catch return null;
    pool.* = MemoryPool.init(block_size, num_blocks) catch {
        allocator.destroy(pool);
        return null;
    };
    
    return pool;
}

export fn zig_memory_pool_destroy(pool: ?*MemoryPool) void {
    if (pool) |p| {
        const allocator = std.heap.c_allocator;
        p.deinit();
        allocator.destroy(p);
    }
}

export fn zig_memory_pool_alloc(pool: ?*MemoryPool, size: usize) ?*anyopaque {
    if (pool) |p| {
        const result = p.alloc(size) catch return null;
        return @ptrCast(result.ptr);
    }
    return null;
}

export fn zig_memory_pool_free(pool: ?*MemoryPool, ptr: ?*anyopaque) void {
    if (pool) |p| {
        if (ptr) |pt| {
            // Convert back to slice - this is approximate since we don't know the size
            const slice = @as([*]u8, @ptrCast(@alignCast(pt)))[0..1]; // Placeholder
            p.free(slice);
        }
    }
}