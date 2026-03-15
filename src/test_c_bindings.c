/*
 * test_c_bindings.c - Simple test program for C bindings with callbacks
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/c_bindings.h"

// Callback function
void test_callback(const char* data, int len, void* user_data) {
    printf("Callback called with data: %.*s\n", len, data);
    if (user_data) {
        printf("User data: %s\n", (const char*)user_data);
    }
}

int main() {
    printf("Testing C bindings for Zig components...\n");
    
    // Test HTTP server creation
    zig_http_server_t* server = zig_http_server_create("127.0.0.1", 8080);
    if (server == NULL) {
        printf("Failed to create HTTP server\n");
        return 1;
    }
    
    printf("Created HTTP server\n");
    
    // Test setting callback
    const char* user_data = "test user data";
    zig_http_server_set_callback(server, test_callback, (void*)user_data);
    printf("Set callback\n");
    
    // Test starting the server
    if (zig_http_server_start(server) != 0) {
        printf("Failed to start HTTP server\n");
        zig_http_server_destroy(server);
        return 1;
    }
    
    printf("Started HTTP server\n");
    
    // Test stopping the server
    zig_http_server_stop(server);
    printf("Stopped HTTP server\n");
    
    // Cleanup
    zig_http_server_destroy(server);
    
    printf("All tests completed successfully!\n");
    return 0;
}