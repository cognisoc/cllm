// config.zig - Simplified compile-time configuration system
const std = @import("std");

// Default configuration values
pub const DEFAULT_CONTEXT_SIZE: u32 = 2048;
pub const DEFAULT_BATCH_SIZE: u32 = 512;
pub const DEFAULT_THREADS: u32 = 4;
pub const DEFAULT_GPU_BACKEND: []const u8 = "cpu";
pub const DEFAULT_HAS_MODEL: bool = false;
pub const DEFAULT_MODEL_PATH: []const u8 = "";
pub const DEFAULT_HTTP_PORT: u16 = 8080;
pub const DEFAULT_HTTP_HOST: []const u8 = "127.0.0.1";
pub const DEFAULT_LOG_LEVEL: []const u8 = "info";

// Compile-time configuration structure
pub const Config = struct {
    context_size: u32 = DEFAULT_CONTEXT_SIZE,
    batch_size: u32 = DEFAULT_BATCH_SIZE,
    threads: u32 = DEFAULT_THREADS,
    gpu_backend: []const u8 = DEFAULT_GPU_BACKEND,
    has_model: bool = DEFAULT_HAS_MODEL,
    model_path: []const u8 = DEFAULT_MODEL_PATH,
    http_port: u16 = DEFAULT_HTTP_PORT,
    http_host: []const u8 = DEFAULT_HTTP_HOST,
    log_level: []const u8 = DEFAULT_LOG_LEVEL,
    
    // Conditional compilation flags - computed at compile time
    pub const USE_GPU = gpu_check();
    pub const USE_CUDA = cuda_check();
    pub const USE_METAL = metal_check();
    pub const USE_VULKAN = vulkan_check();
    
    // Helper functions for compile-time checks
    fn gpu_check() bool {
        return DEFAULT_GPU_BACKEND.len > 0 and DEFAULT_GPU_BACKEND[0] != 'c'; // not "cpu"
    }
    
    fn cuda_check() bool {
        return DEFAULT_GPU_BACKEND.len == 4 and 
               DEFAULT_GPU_BACKEND[0] == 'c' and 
               DEFAULT_GPU_BACKEND[1] == 'u' and 
               DEFAULT_GPU_BACKEND[2] == 'd' and 
               DEFAULT_GPU_BACKEND[3] == 'a';
    }
    
    fn metal_check() bool {
        return DEFAULT_GPU_BACKEND.len == 5 and 
               DEFAULT_GPU_BACKEND[0] == 'm' and 
               DEFAULT_GPU_BACKEND[1] == 'e' and 
               DEFAULT_GPU_BACKEND[2] == 't' and 
               DEFAULT_GPU_BACKEND[3] == 'a' and 
               DEFAULT_GPU_BACKEND[4] == 'l';
    }
    
    fn vulkan_check() bool {
        return DEFAULT_GPU_BACKEND.len == 6 and 
               DEFAULT_GPU_BACKEND[0] == 'v' and 
               DEFAULT_GPU_BACKEND[1] == 'u' and 
               DEFAULT_GPU_BACKEND[2] == 'l' and 
               DEFAULT_GPU_BACKEND[3] == 'k' and 
               DEFAULT_GPU_BACKEND[4] == 'a' and 
               DEFAULT_GPU_BACKEND[5] == 'n';
    }
};

// Configuration loader
pub const ConfigLoader = struct {
    allocator: std.mem.Allocator,
    
    pub fn init(allocator: std.mem.Allocator) ConfigLoader {
        return ConfigLoader{
            .allocator = allocator,
        };
    }
    
    pub fn loadConfig(self: *ConfigLoader) !Config {
        _ = self; // Fixed: Remove unused parameter warning
        // In a real implementation, this would load configuration from:
        // 1. Compile-time embedded JSON
        // 2. Environment variables
        // 3. Command-line arguments
        // 4. Default values
        
        return Config{};
    }
    
    pub fn validateConfig(self: *ConfigLoader, config: Config) !bool {
        _ = self; // Fixed: Remove unused parameter warning
        _ = config; // Fixed: Remove unused parameter warning
        // In a real implementation, this would validate configuration
        // For now, we'll assume all configurations are valid
        return true;
    }
};

// JSON configuration parser
pub const JsonConfigParser = struct {
    allocator: std.mem.Allocator,
    
    pub fn init(allocator: std.mem.Allocator) JsonConfigParser {
        return JsonConfigParser{
            .allocator = allocator,
        };
    }
    
    pub fn generateJsonConfig(self: *JsonConfigParser, config: Config) ![]const u8 {
        _ = config; // Fixed: Remove unused parameter warning
        // In a real implementation, this would generate JSON configuration
        // For now, we'll return a simple JSON string
        return try std.fmt.allocPrint(self.allocator, 
            "{{\"context_size\": {}, \"batch_size\": {}, \"threads\": {}, \"gpu_backend\": \"{s}\", \"has_model\": {}}}",
            .{Config.DEFAULT_CONTEXT_SIZE, Config.DEFAULT_BATCH_SIZE, Config.DEFAULT_THREADS, 
              Config.DEFAULT_GPU_BACKEND, Config.DEFAULT_HAS_MODEL});
    }
};

// Configuration validator
pub const ConfigValidator = struct {
    pub fn validateContextSize(context_size: u32) bool {
        // Validate context size is reasonable
        return context_size > 0 and context_size <= 32768;
    }
    
    pub fn validateBatchSize(batch_size: u32) bool {
        // Validate batch size is reasonable
        return batch_size > 0 and batch_size <= 4096;
    }
    
    pub fn validateThreads(threads: u32) bool {
        // Validate thread count is reasonable
        return threads > 0 and threads <= 128;
    }
    
    pub fn validateGpuBackend(gpu_backend: []const u8) bool {
        // Validate GPU backend is supported
        const supported_backends = [_][]const u8{
            "cpu", "cuda", "metal", "vulkan"
        };
        
        for (supported_backends) |backend| {
            if (std.mem.eql(u8, gpu_backend, backend)) {
                return true;
            }
        }
        
        return false;
    }
    
    pub fn validateHttpPort(port: u16) bool {
        // Validate HTTP port is reasonable
        return port > 0 and port < 65535;
    }
    
    pub fn validateHttpHost(host: []const u8) bool {
        // Validate HTTP host is reasonable
        return host.len > 0 and host.len <= 255;
    }
    
    pub fn validateLogLevel(log_level: []const u8) bool {
        // Validate log level is supported
        const supported_levels = [_][]const u8{
            "debug", "info", "warn", "error"
        };
        
        for (supported_levels) |level| {
            if (std.mem.eql(u8, log_level, level)) {
                return true;
            }
        }
        
        return false;
    }
};