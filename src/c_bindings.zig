// c_bindings.zig - Simple Zig implementation of C bindings with callbacks and memory management
const std = @import("std");
const builtin = @import("builtin");
const memory = @import("memory_management.zig");

// HTTP server implementation
pub const HttpServer = struct {
    host: []const u8,
    port: u16,
    callback: ?*const fn (data: [*c]const u8, len: c_int, user_data: ?*anyopaque) callconv(.c) void,
    user_data: ?*anyopaque,
    
    pub fn init(host: []const u8, port: u16) HttpServer {
        return HttpServer{
            .host = host,
            .port = port,
            .callback = null,
            .user_data = null,
        };
    }
    
    pub fn setCallback(self: *HttpServer, callback: ?*const fn (data: [*c]const u8, len: c_int, user_data: ?*anyopaque) callconv(.c) void, user_data: ?*anyopaque) void {
        self.callback = callback;
        self.user_data = user_data;
    }
    
    pub fn start(self: *HttpServer) void {
        std.debug.print("Starting HTTP server on {s}:{d}\n", .{self.host, self.port});
        
        // Simulate calling the callback
        if (self.callback) |cb| {
            const test_data = "HTTP server started";
            cb(test_data, @intCast(test_data.len), self.user_data);
        }
    }
    
    pub fn stop(_: *HttpServer) void {
        std.debug.print("Stopping HTTP server\n", .{});
    }
};

// C-compatible wrapper functions
export fn zig_http_server_create(host: [*c]const u8, port: u16) ?*HttpServer {
    const allocator = std.heap.c_allocator;
    const host_slice = std.mem.span(host);
    
    const server = allocator.create(HttpServer) catch return null;
    server.* = HttpServer.init(host_slice, port);
    
    return server;
}

export fn zig_http_server_destroy(server: ?*HttpServer) void {
    if (server) |s| {
        const allocator = std.heap.c_allocator;
        allocator.destroy(s);
    }
}

export fn zig_http_server_start(server: ?*HttpServer) c_int {
    if (server) |s| {
        s.start();
        return 0;
    }
    return -1;
}

export fn zig_http_server_stop(server: ?*HttpServer) void {
    if (server) |s| {
        s.stop();
    }
}

export fn zig_http_server_set_callback(
    server: ?*HttpServer,
    callback: ?*const fn (data: [*c]const u8, len: c_int, user_data: ?*anyopaque) callconv(.c) void,
    user_data: ?*anyopaque
) void {
    if (server) |s| {
        s.setCallback(callback, user_data);
    }
}