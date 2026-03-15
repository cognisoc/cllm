// config.zig - Configuration for the unikernel
const std = @import("std");

pub const Config = struct {
    pub const CONTEXT_SIZE: u32 = 2048;
    pub const BATCH_SIZE: u32 = 512;
    pub const THREADS: u32 = 4;
    pub const GPU_BACKEND: []const u8 = "cuda";
    pub const HAS_MODEL: bool = false;
    
    // Conditional compilation flags - computed at compile time
    pub const USE_GPU: bool = comptime gpu_check();
    pub const USE_CUDA: bool = comptime cuda_check();
    pub const USE_METAL: bool = comptime metal_check();
    pub const USE_VULKAN: bool = comptime vulkan_check();
    
    // Helper functions for compile-time checks
    fn gpu_check() bool {
        return GPU_BACKEND.len > 0 and GPU_BACKEND[0] != 'c'; // not "cpu"
    }
    
    fn cuda_check() bool {
        return GPU_BACKEND.len == 4 and 
               GPU_BACKEND[0] == 'c' and 
               GPU_BACKEND[1] == 'u' and 
               GPU_BACKEND[2] == 'd' and 
               GPU_BACKEND[3] == 'a';
    }
    
    fn metal_check() bool {
        return GPU_BACKEND.len == 5 and 
               GPU_BACKEND[0] == 'm' and 
               GPU_BACKEND[1] == 'e' and 
               GPU_BACKEND[2] == 't' and 
               GPU_BACKEND[3] == 'a' and 
               GPU_BACKEND[4] == 'l';
    }
    
    fn vulkan_check() bool {
        return GPU_BACKEND.len == 6 and 
               GPU_BACKEND[0] == 'v' and 
               GPU_BACKEND[1] == 'u' and 
               GPU_BACKEND[2] == 'l' and 
               GPU_BACKEND[3] == 'k' and 
               GPU_BACKEND[4] == 'a' and 
               GPU_BACKEND[5] == 'n';
    }
};