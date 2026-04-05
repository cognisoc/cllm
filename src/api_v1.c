/*
 * api_v1.c - llama.cpp-compatible API v1 handlers
 */

#include "api_v1.h"
#include "api_types.h"
#include "json.h"
#include "string.h"
#include "kernel.h"

#include <stdbool.h>

// Global API context
typedef struct {
    llm_context_t* llm_ctx;
    bool model_loaded;
    uint64_t request_counter;
    int64_t created_time;
} api_context_t;

static api_context_t g_api_ctx = {0};

// Model ID for the embedded model
static const char* MODEL_ID = "embedded-llama";

// ============================================================================
// Helper Functions
// ============================================================================

// Set JSON content type header
static void set_json_headers(http_response_t* response) {
    strncpy(response->headers, "Content-Type: application/json\r\n", sizeof(response->headers));
}

// Generate request ID
static void generate_request_id(char* buf, size_t size, const char* prefix) {
    g_api_ctx.request_counter++;
    snprintf(buf, size, "%s-%016llx",
             prefix, (unsigned long long)g_api_ctx.request_counter);
}

// Check if request method is POST
static bool is_post_request(http_request_t* request) {
    return strncmp(request->method, "POST", 5) == 0;
}

// Check if request method is GET
static bool is_get_request(http_request_t* request) {
    return strncmp(request->method, "GET", 4) == 0;
}

// Build error response
static int build_error_response(http_response_t* response, int status_code,
                                const char* message, const char* type,
                                const char* param, const char* code) {
    response->status_code = status_code;
    set_json_headers(response);

    json_writer_t w;
    json_writer_init(&w, response->body, sizeof(response->body));

    json_write_object_start(&w);
    json_write_key(&w, "error");
    json_write_object_start(&w);
    json_write_kv_string(&w, "message", message);
    json_write_kv_string(&w, "type", type);
    if (param && param[0]) {
        json_write_kv_string(&w, "param", param);
    } else {
        json_write_kv_null(&w, "param");
    }
    if (code && code[0]) {
        json_write_kv_string(&w, "code", code);
    } else {
        json_write_kv_null(&w, "code");
    }
    json_write_object_end(&w);
    json_write_object_end(&w);

    response->body_length = json_writer_length(&w);
    return 0;
}

// ============================================================================
// Request Parsing
// ============================================================================

// Parse completion request from JSON
static int parse_completion_request(const char* body, size_t len, completion_request_t* req) {
    completion_request_init(req);

    json_parser_t p;
    json_parser_init(&p, body, len);

    if (json_next_token(&p) != JSON_TOK_LBRACE) {
        return -1;
    }

    json_next_token(&p);
    while (p.current != JSON_TOK_RBRACE && p.current != JSON_TOK_EOF) {
        if (p.current != JSON_TOK_STRING) return -1;

        char key[64];
        if (json_parse_string(&p, key, sizeof(key)) != 0) return -1;

        if (p.current != JSON_TOK_COLON) return -1;
        json_next_token(&p);

        if (strcmp(key, "prompt") == 0) {
            if (json_parse_string(&p, req->prompt, sizeof(req->prompt)) != 0) return -1;
        } else if (strcmp(key, "model") == 0) {
            if (json_parse_string(&p, req->model, sizeof(req->model)) != 0) return -1;
        } else if (strcmp(key, "max_tokens") == 0) {
            if (json_parse_int(&p, &req->max_tokens) != 0) return -1;
        } else if (strcmp(key, "temperature") == 0) {
            double v;
            if (json_parse_number(&p, &v) != 0) return -1;
            req->temperature = (float)v;
        } else if (strcmp(key, "top_p") == 0) {
            double v;
            if (json_parse_number(&p, &v) != 0) return -1;
            req->top_p = (float)v;
        } else if (strcmp(key, "top_k") == 0) {
            if (json_parse_int(&p, &req->top_k) != 0) return -1;
        } else if (strcmp(key, "frequency_penalty") == 0) {
            double v;
            if (json_parse_number(&p, &v) != 0) return -1;
            req->frequency_penalty = (float)v;
        } else if (strcmp(key, "presence_penalty") == 0) {
            double v;
            if (json_parse_number(&p, &v) != 0) return -1;
            req->presence_penalty = (float)v;
        } else if (strcmp(key, "stream") == 0) {
            if (json_parse_bool(&p, &req->stream) != 0) return -1;
        } else if (strcmp(key, "seed") == 0) {
            if (json_parse_int(&p, &req->seed) != 0) return -1;
        } else if (strcmp(key, "echo") == 0) {
            if (json_parse_bool(&p, &req->echo) != 0) return -1;
        } else if (strcmp(key, "n") == 0) {
            if (json_parse_int(&p, &req->n) != 0) return -1;
        } else {
            // Skip unknown fields
            if (json_skip_value(&p) != 0) return -1;
        }

        if (p.current == JSON_TOK_COMMA) {
            json_next_token(&p);
        }
    }

    return 0;
}

// Parse chat completion request
static int parse_chat_request(const char* body, size_t len, chat_completion_request_t* req) {
    chat_completion_request_init(req);

    json_parser_t p;
    json_parser_init(&p, body, len);

    if (json_next_token(&p) != JSON_TOK_LBRACE) {
        return -1;
    }

    json_next_token(&p);
    while (p.current != JSON_TOK_RBRACE && p.current != JSON_TOK_EOF) {
        if (p.current != JSON_TOK_STRING) return -1;

        char key[64];
        if (json_parse_string(&p, key, sizeof(key)) != 0) return -1;

        if (p.current != JSON_TOK_COLON) return -1;
        json_next_token(&p);

        if (strcmp(key, "messages") == 0) {
            if (p.current != JSON_TOK_LBRACKET) return -1;
            json_next_token(&p);

            while (p.current != JSON_TOK_RBRACKET && p.current != JSON_TOK_EOF) {
                if (req->message_count >= MAX_CHAT_MESSAGES) break;

                if (p.current != JSON_TOK_LBRACE) return -1;
                json_next_token(&p);

                chat_message_t* msg = &req->messages[req->message_count];
                msg->role[0] = '\0';
                msg->content[0] = '\0';

                while (p.current != JSON_TOK_RBRACE && p.current != JSON_TOK_EOF) {
                    if (p.current != JSON_TOK_STRING) return -1;
                    char mkey[32];
                    if (json_parse_string(&p, mkey, sizeof(mkey)) != 0) return -1;

                    if (p.current != JSON_TOK_COLON) return -1;
                    json_next_token(&p);

                    if (strcmp(mkey, "role") == 0) {
                        if (json_parse_string(&p, msg->role, sizeof(msg->role)) != 0) return -1;
                    } else if (strcmp(mkey, "content") == 0) {
                        if (json_parse_string(&p, msg->content, sizeof(msg->content)) != 0) return -1;
                    } else {
                        if (json_skip_value(&p) != 0) return -1;
                    }

                    if (p.current == JSON_TOK_COMMA) {
                        json_next_token(&p);
                    }
                }

                if (p.current == JSON_TOK_RBRACE) {
                    json_next_token(&p);
                    req->message_count++;
                }

                if (p.current == JSON_TOK_COMMA) {
                    json_next_token(&p);
                }
            }

            if (p.current == JSON_TOK_RBRACKET) {
                json_next_token(&p);
            }
        } else if (strcmp(key, "model") == 0) {
            if (json_parse_string(&p, req->model, sizeof(req->model)) != 0) return -1;
        } else if (strcmp(key, "max_tokens") == 0) {
            if (json_parse_int(&p, &req->max_tokens) != 0) return -1;
        } else if (strcmp(key, "temperature") == 0) {
            double v;
            if (json_parse_number(&p, &v) != 0) return -1;
            req->temperature = (float)v;
        } else if (strcmp(key, "top_p") == 0) {
            double v;
            if (json_parse_number(&p, &v) != 0) return -1;
            req->top_p = (float)v;
        } else if (strcmp(key, "stream") == 0) {
            if (json_parse_bool(&p, &req->stream) != 0) return -1;
        } else {
            if (json_skip_value(&p) != 0) return -1;
        }

        if (p.current == JSON_TOK_COMMA) {
            json_next_token(&p);
        }
    }

    return 0;
}

// Parse tokenize request
static int parse_tokenize_request(const char* body, size_t len, tokenize_request_t* req) {
    tokenize_request_init(req);

    json_parser_t p;
    json_parser_init(&p, body, len);

    if (json_next_token(&p) != JSON_TOK_LBRACE) {
        return -1;
    }

    json_next_token(&p);
    while (p.current != JSON_TOK_RBRACE && p.current != JSON_TOK_EOF) {
        if (p.current != JSON_TOK_STRING) return -1;

        char key[64];
        if (json_parse_string(&p, key, sizeof(key)) != 0) return -1;

        if (p.current != JSON_TOK_COLON) return -1;
        json_next_token(&p);

        if (strcmp(key, "content") == 0) {
            if (json_parse_string(&p, req->content, sizeof(req->content)) != 0) return -1;
        } else if (strcmp(key, "add_special") == 0) {
            if (json_parse_bool(&p, &req->add_special) != 0) return -1;
        } else {
            if (json_skip_value(&p) != 0) return -1;
        }

        if (p.current == JSON_TOK_COMMA) {
            json_next_token(&p);
        }
    }

    return 0;
}

// Parse detokenize request
static int parse_detokenize_request(const char* body, size_t len, detokenize_request_t* req) {
    detokenize_request_init(req);

    json_parser_t p;
    json_parser_init(&p, body, len);

    if (json_next_token(&p) != JSON_TOK_LBRACE) {
        return -1;
    }

    json_next_token(&p);
    while (p.current != JSON_TOK_RBRACE && p.current != JSON_TOK_EOF) {
        if (p.current != JSON_TOK_STRING) return -1;

        char key[64];
        if (json_parse_string(&p, key, sizeof(key)) != 0) return -1;

        if (p.current != JSON_TOK_COLON) return -1;
        json_next_token(&p);

        if (strcmp(key, "tokens") == 0) {
            if (p.current != JSON_TOK_LBRACKET) return -1;
            json_next_token(&p);

            while (p.current != JSON_TOK_RBRACKET && p.current != JSON_TOK_EOF) {
                if (req->token_count >= MAX_TOKENS) break;

                if (p.current == JSON_TOK_NUMBER) {
                    int token;
                    if (json_parse_int(&p, &token) != 0) return -1;
                    req->tokens[req->token_count++] = token;
                } else {
                    return -1;
                }

                if (p.current == JSON_TOK_COMMA) {
                    json_next_token(&p);
                }
            }

            if (p.current == JSON_TOK_RBRACKET) {
                json_next_token(&p);
            }
        } else {
            if (json_skip_value(&p) != 0) return -1;
        }

        if (p.current == JSON_TOK_COMMA) {
            json_next_token(&p);
        }
    }

    return 0;
}

// Parse embedding request
static int parse_embedding_request(const char* body, size_t len, embedding_request_t* req) {
    embedding_request_init(req);

    json_parser_t p;
    json_parser_init(&p, body, len);

    if (json_next_token(&p) != JSON_TOK_LBRACE) {
        return -1;
    }

    json_next_token(&p);
    while (p.current != JSON_TOK_RBRACE && p.current != JSON_TOK_EOF) {
        if (p.current != JSON_TOK_STRING) return -1;

        char key[64];
        if (json_parse_string(&p, key, sizeof(key)) != 0) return -1;

        if (p.current != JSON_TOK_COLON) return -1;
        json_next_token(&p);

        if (strcmp(key, "input") == 0) {
            if (json_parse_string(&p, req->input, sizeof(req->input)) != 0) return -1;
        } else if (strcmp(key, "model") == 0) {
            if (json_parse_string(&p, req->model, sizeof(req->model)) != 0) return -1;
        } else {
            if (json_skip_value(&p) != 0) return -1;
        }

        if (p.current == JSON_TOK_COMMA) {
            json_next_token(&p);
        }
    }

    return 0;
}

// ============================================================================
// API Initialization
// ============================================================================

int api_v1_init(llm_context_t* llm_ctx) {
    g_api_ctx.llm_ctx = llm_ctx;
    g_api_ctx.model_loaded = (llm_ctx != NULL);
    g_api_ctx.request_counter = 0;
    g_api_ctx.created_time = 1234567890; // Placeholder timestamp
    serial_write("API v1: Initialized\r\n");
    return 0;
}

// ============================================================================
// Endpoint Handlers
// ============================================================================

// POST /v1/completions
int handle_v1_completions(http_request_t* request, http_response_t* response) {
    serial_write("API v1: /v1/completions\r\n");

    if (!is_post_request(request)) {
        return build_error_response(response, 405, "Method not allowed",
                                    "invalid_request_error", "", "method_not_allowed");
    }

    if (request->body_length == 0) {
        return build_error_response(response, 400, "Request body is empty",
                                    "invalid_request_error", "", "empty_body");
    }

    completion_request_t req;
    if (parse_completion_request(request->body, request->body_length, &req) != 0) {
        return build_error_response(response, 400, "Invalid JSON",
                                    "invalid_request_error", "", "json_parse_error");
    }

    if (req.prompt[0] == '\0') {
        return build_error_response(response, 400, "Missing required field: prompt",
                                    "invalid_request_error", "prompt", "missing_field");
    }

    // Perform tokenization and generation
    int tokens[MAX_TOKENS];
    int n_tokens = 0;
    char generated_text[MAX_PROMPT_LENGTH] = "";

    if (g_api_ctx.llm_ctx) {
        n_tokens = llm_tokenize(g_api_ctx.llm_ctx, req.prompt, tokens, MAX_TOKENS);
        if (n_tokens < 0) n_tokens = 0;

        // Placeholder generation - in real impl, would call llm_eval and llm_sample
        // For now, generate a simple response
        strncpy(generated_text, " [completion placeholder]", sizeof(generated_text));
    } else {
        strncpy(generated_text, " [model not loaded]", sizeof(generated_text));
    }

    // Build response
    response->status_code = 200;
    set_json_headers(response);

    char request_id[64];
    generate_request_id(request_id, sizeof(request_id), "cmpl");

    json_writer_t w;
    json_writer_init(&w, response->body, sizeof(response->body));

    json_write_object_start(&w);
    json_write_kv_string(&w, "id", request_id);
    json_write_kv_string(&w, "object", "text_completion");
    json_write_kv_int64(&w, "created", g_api_ctx.created_time);
    json_write_kv_string(&w, "model", MODEL_ID);

    // Choices array
    json_write_key(&w, "choices");
    json_write_array_start(&w);
    json_write_object_start(&w);
    json_write_kv_string(&w, "text", generated_text);
    json_write_kv_int(&w, "index", 0);
    json_write_kv_null(&w, "logprobs");
    json_write_kv_string(&w, "finish_reason", "stop");
    json_write_object_end(&w);
    json_write_array_end(&w);

    // Usage
    json_write_key(&w, "usage");
    json_write_object_start(&w);
    json_write_kv_int(&w, "prompt_tokens", n_tokens);
    json_write_kv_int(&w, "completion_tokens", 1);
    json_write_kv_int(&w, "total_tokens", n_tokens + 1);
    json_write_object_end(&w);

    json_write_object_end(&w);

    response->body_length = json_writer_length(&w);
    return 0;
}

// POST /v1/chat/completions
int handle_v1_chat_completions(http_request_t* request, http_response_t* response) {
    serial_write("API v1: /v1/chat/completions\r\n");

    if (!is_post_request(request)) {
        return build_error_response(response, 405, "Method not allowed",
                                    "invalid_request_error", "", "method_not_allowed");
    }

    if (request->body_length == 0) {
        return build_error_response(response, 400, "Request body is empty",
                                    "invalid_request_error", "", "empty_body");
    }

    chat_completion_request_t req;
    if (parse_chat_request(request->body, request->body_length, &req) != 0) {
        return build_error_response(response, 400, "Invalid JSON",
                                    "invalid_request_error", "", "json_parse_error");
    }

    if (req.message_count == 0) {
        return build_error_response(response, 400, "Missing required field: messages",
                                    "invalid_request_error", "messages", "missing_field");
    }

    // Placeholder response
    const char* assistant_response = "Hello! I'm a placeholder response from the CLLM unikernel.";

    // Count tokens in all messages (simplified)
    int prompt_tokens = 0;
    for (int i = 0; i < req.message_count; i++) {
        prompt_tokens += (int)strlen(req.messages[i].content);
    }
    int completion_tokens = (int)strlen(assistant_response);

    // Build response
    response->status_code = 200;
    set_json_headers(response);

    char request_id[64];
    generate_request_id(request_id, sizeof(request_id), "chatcmpl");

    json_writer_t w;
    json_writer_init(&w, response->body, sizeof(response->body));

    json_write_object_start(&w);
    json_write_kv_string(&w, "id", request_id);
    json_write_kv_string(&w, "object", "chat.completion");
    json_write_kv_int64(&w, "created", g_api_ctx.created_time);
    json_write_kv_string(&w, "model", MODEL_ID);

    // Choices array
    json_write_key(&w, "choices");
    json_write_array_start(&w);
    json_write_object_start(&w);
    json_write_kv_int(&w, "index", 0);

    // Message object
    json_write_key(&w, "message");
    json_write_object_start(&w);
    json_write_kv_string(&w, "role", "assistant");
    json_write_kv_string(&w, "content", assistant_response);
    json_write_object_end(&w);

    json_write_kv_string(&w, "finish_reason", "stop");
    json_write_object_end(&w);
    json_write_array_end(&w);

    // Usage
    json_write_key(&w, "usage");
    json_write_object_start(&w);
    json_write_kv_int(&w, "prompt_tokens", prompt_tokens);
    json_write_kv_int(&w, "completion_tokens", completion_tokens);
    json_write_kv_int(&w, "total_tokens", prompt_tokens + completion_tokens);
    json_write_object_end(&w);

    json_write_object_end(&w);

    response->body_length = json_writer_length(&w);
    return 0;
}

// POST /v1/embeddings
int handle_v1_embeddings(http_request_t* request, http_response_t* response) {
    serial_write("API v1: /v1/embeddings\r\n");

    if (!is_post_request(request)) {
        return build_error_response(response, 405, "Method not allowed",
                                    "invalid_request_error", "", "method_not_allowed");
    }

    if (request->body_length == 0) {
        return build_error_response(response, 400, "Request body is empty",
                                    "invalid_request_error", "", "empty_body");
    }

    embedding_request_t req;
    if (parse_embedding_request(request->body, request->body_length, &req) != 0) {
        return build_error_response(response, 400, "Invalid JSON",
                                    "invalid_request_error", "", "json_parse_error");
    }

    if (req.input[0] == '\0') {
        return build_error_response(response, 400, "Missing required field: input",
                                    "invalid_request_error", "input", "missing_field");
    }

    // Placeholder embedding (small fixed-size for demo)
    int embedding_dim = 8;
    float embedding[8] = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f};
    int prompt_tokens = (int)strlen(req.input);

    // Build response
    response->status_code = 200;
    set_json_headers(response);

    json_writer_t w;
    json_writer_init(&w, response->body, sizeof(response->body));

    json_write_object_start(&w);
    json_write_kv_string(&w, "object", "list");

    // Data array
    json_write_key(&w, "data");
    json_write_array_start(&w);
    json_write_object_start(&w);
    json_write_kv_string(&w, "object", "embedding");
    json_write_kv_int(&w, "index", 0);

    json_write_key(&w, "embedding");
    json_write_float_array(&w, embedding, embedding_dim);

    json_write_object_end(&w);
    json_write_array_end(&w);

    json_write_kv_string(&w, "model", MODEL_ID);

    // Usage
    json_write_key(&w, "usage");
    json_write_object_start(&w);
    json_write_kv_int(&w, "prompt_tokens", prompt_tokens);
    json_write_kv_int(&w, "total_tokens", prompt_tokens);
    json_write_object_end(&w);

    json_write_object_end(&w);

    response->body_length = json_writer_length(&w);
    return 0;
}

// GET /v1/models
int handle_v1_models(http_request_t* request, http_response_t* response) {
    serial_write("API v1: /v1/models\r\n");

    if (!is_get_request(request)) {
        return build_error_response(response, 405, "Method not allowed",
                                    "invalid_request_error", "", "method_not_allowed");
    }

    response->status_code = 200;
    set_json_headers(response);

    json_writer_t w;
    json_writer_init(&w, response->body, sizeof(response->body));

    json_write_object_start(&w);
    json_write_kv_string(&w, "object", "list");

    json_write_key(&w, "data");
    json_write_array_start(&w);

    // Model entry
    json_write_object_start(&w);
    json_write_kv_string(&w, "id", MODEL_ID);
    json_write_kv_string(&w, "object", "model");
    json_write_kv_int64(&w, "created", g_api_ctx.created_time);
    json_write_kv_string(&w, "owned_by", "cllm-unikernel");
    json_write_object_end(&w);

    json_write_array_end(&w);
    json_write_object_end(&w);

    response->body_length = json_writer_length(&w);
    return 0;
}

// POST /tokenize
int handle_tokenize(http_request_t* request, http_response_t* response) {
    serial_write("API v1: /tokenize\r\n");

    if (!is_post_request(request)) {
        return build_error_response(response, 405, "Method not allowed",
                                    "invalid_request_error", "", "method_not_allowed");
    }

    if (request->body_length == 0) {
        return build_error_response(response, 400, "Request body is empty",
                                    "invalid_request_error", "", "empty_body");
    }

    tokenize_request_t req;
    if (parse_tokenize_request(request->body, request->body_length, &req) != 0) {
        return build_error_response(response, 400, "Invalid JSON",
                                    "invalid_request_error", "", "json_parse_error");
    }

    if (req.content[0] == '\0') {
        return build_error_response(response, 400, "Missing required field: content",
                                    "invalid_request_error", "content", "missing_field");
    }

    // Tokenize
    int tokens[MAX_TOKENS];
    int n_tokens = 0;

    if (g_api_ctx.llm_ctx) {
        n_tokens = llm_tokenize(g_api_ctx.llm_ctx, req.content, tokens, MAX_TOKENS);
        if (n_tokens < 0) n_tokens = 0;
    } else {
        // Fallback: character-level tokenization
        size_t len = strlen(req.content);
        n_tokens = (int)(len < MAX_TOKENS ? len : MAX_TOKENS);
        for (int i = 0; i < n_tokens; i++) {
            tokens[i] = (int)(unsigned char)req.content[i];
        }
    }

    // Build response
    response->status_code = 200;
    set_json_headers(response);

    json_writer_t w;
    json_writer_init(&w, response->body, sizeof(response->body));

    json_write_object_start(&w);

    json_write_key(&w, "tokens");
    json_write_int_array(&w, tokens, n_tokens);

    json_write_object_end(&w);

    response->body_length = json_writer_length(&w);
    return 0;
}

// POST /detokenize
int handle_detokenize(http_request_t* request, http_response_t* response) {
    serial_write("API v1: /detokenize\r\n");

    if (!is_post_request(request)) {
        return build_error_response(response, 405, "Method not allowed",
                                    "invalid_request_error", "", "method_not_allowed");
    }

    if (request->body_length == 0) {
        return build_error_response(response, 400, "Request body is empty",
                                    "invalid_request_error", "", "empty_body");
    }

    detokenize_request_t req;
    if (parse_detokenize_request(request->body, request->body_length, &req) != 0) {
        return build_error_response(response, 400, "Invalid JSON",
                                    "invalid_request_error", "", "json_parse_error");
    }

    if (req.token_count == 0) {
        return build_error_response(response, 400, "Missing required field: tokens",
                                    "invalid_request_error", "tokens", "missing_field");
    }

    // Detokenize
    char text[MAX_PROMPT_LENGTH];
    text[0] = '\0';

    if (g_api_ctx.llm_ctx) {
        llm_detokenize(g_api_ctx.llm_ctx, req.tokens, req.token_count, text, sizeof(text));
    } else {
        // Fallback: ASCII conversion
        int len = req.token_count < (int)(sizeof(text) - 1) ? req.token_count : (int)(sizeof(text) - 1);
        for (int i = 0; i < len; i++) {
            text[i] = (char)(req.tokens[i] & 0x7F);
        }
        text[len] = '\0';
    }

    // Build response
    response->status_code = 200;
    set_json_headers(response);

    json_writer_t w;
    json_writer_init(&w, response->body, sizeof(response->body));

    json_write_object_start(&w);
    json_write_kv_string(&w, "content", text);
    json_write_object_end(&w);

    response->body_length = json_writer_length(&w);
    return 0;
}
