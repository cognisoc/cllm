/*
 * api.c - Simple API handlers for our LLM server
 */

#include "network.h"
#include "c_model_interface.h"
#include "api_v1.h"
#include "string.h"
#include "kernel.h"
#include "safestring.h"

// Handle health check endpoint
int handle_health_check(http_response_t* response) {
    response->status_code = 200;
    safe_strcpy(response->headers, sizeof(response->headers),
                "Content-Type: application/json\r\n");
    safe_strcpy(response->body, sizeof(response->body), "{\"status\": \"ok\"}");
    response->body_length = strlen(response->body);
    return 0;
}

// Handle model metadata endpoint
int handle_model_metadata(http_response_t* response) {
    response->status_code = 200;
    safe_strcpy(response->headers, sizeof(response->headers),
                "Content-Type: application/json\r\n");

    // Build a small JSON with metadata from the embedded model
    model_metadata_t meta = c_get_model_metadata();
    char arch[33];
    size_t i = 0;
    for (; i < sizeof(arch) - 1 && i < sizeof(meta.architecture) && meta.architecture[i] != '\0'; ++i) {
        arch[i] = meta.architecture[i];
    }
    arch[i] = '\0';

    int n = snprintf(
        response->body,
        sizeof(response->body),
        "{\"magic\":\"%c%c%c%c\",\"version\":%u,\"tensor_count\":%u,\"kv_count\":%u,\"architecture\":\"%s\",\"context_length\":%u}",
        meta.magic[0], meta.magic[1], meta.magic[2], meta.magic[3],
        meta.version, meta.tensor_count, meta.kv_count, arch, meta.context_length
    );
    if (n < 0 || (size_t)n >= sizeof(response->body)) {
        response->status_code = 500;
        safe_strcpy(response->body, sizeof(response->body),
                    "{\"error\":\"failed to format metadata\"}");
    }
    response->body_length = strlen(response->body);
    return 0;
}

// Handle model loading endpoint
int handle_load_model(http_response_t* response) {
    response->status_code = 200;
    safe_strcpy(response->headers, sizeof(response->headers),
                "Content-Type: application/json\r\n");
    safe_strcpy(response->body, sizeof(response->body),
                "{\"status\": \"loading\", \"message\": \"Model loading not yet implemented\"}");
    response->body_length = strlen(response->body);
    return 0;
}

// Handle text completion endpoint
int handle_completion(http_response_t* response) {
    response->status_code = 200;
    safe_strcpy(response->headers, sizeof(response->headers),
                "Content-Type: application/json\r\n");
    safe_strcpy(response->body, sizeof(response->body),
                "{\"status\": \"generating\", \"message\": \"Text completion not yet implemented\"}");
    response->body_length = strlen(response->body);
    return 0;
}

// Route HTTP requests to appropriate handlers
int route_request(http_request_t* request, http_response_t* response) {
    // Default headers
    safe_strcpy(response->headers, sizeof(response->headers),
                "Content-Type: application/json\r\n");

    // V1 API routes (llama.cpp compatible)
    if (strncmp(request->path, "/v1/completions", 15) == 0) {
        return handle_v1_completions(request, response);
    }
    if (strncmp(request->path, "/v1/chat/completions", 20) == 0) {
        return handle_v1_chat_completions(request, response);
    }
    if (strncmp(request->path, "/v1/embeddings", 14) == 0) {
        return handle_v1_embeddings(request, response);
    }
    if (strncmp(request->path, "/v1/models", 10) == 0) {
        return handle_v1_models(request, response);
    }
    if (strncmp(request->path, "/tokenize", 9) == 0) {
        return handle_tokenize(request, response);
    }
    if (strncmp(request->path, "/detokenize", 11) == 0) {
        return handle_detokenize(request, response);
    }

    // Legacy routes
    if (strncmp(request->path, "/health", 8) == 0) {
        int rc = handle_health_check(response);
        response->body_length = strlen(response->body);
        return rc;
    }
    if (strncmp(request->path, "/model/metadata", 15) == 0) {
        int rc = handle_model_metadata(response);
        response->body_length = strlen(response->body);
        return rc;
    }
    if (strncmp(request->path, "/model/load", 12) == 0) {
        int rc = handle_load_model(response);
        response->body_length = strlen(response->body);
        return rc;
    }
    if (strncmp(request->path, "/completion", 11) == 0) {
        int rc = handle_completion(response);
        response->body_length = strlen(response->body);
        return rc;
    }

    // Not found
    response->status_code = 404;
    safe_strcpy(response->body, sizeof(response->body), "{\"error\":\"not found\"}");
    response->body_length = strlen(response->body);
    return 0;
}
