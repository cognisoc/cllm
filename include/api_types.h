/*
 * api_types.h - API request/response type definitions for llama.cpp compatibility
 */

#ifndef API_TYPES_H
#define API_TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Maximum limits
#define MAX_PROMPT_LENGTH    4096
#define MAX_TOKENS           2048
#define MAX_STOP_SEQUENCES   4
#define MAX_STOP_LENGTH      64
#define MAX_CHAT_MESSAGES    32
#define MAX_MODEL_ID_LENGTH  64
#define MAX_EMBEDDING_DIM    4096

// Default values
#define DEFAULT_MAX_TOKENS      256
#define DEFAULT_TEMPERATURE     0.8f
#define DEFAULT_TOP_P           0.95f
#define DEFAULT_TOP_K           40
#define DEFAULT_FREQ_PENALTY    0.0f
#define DEFAULT_PRES_PENALTY    0.0f
#define DEFAULT_SEED            -1

// ============================================================================
// Request Types
// ============================================================================

// OpenAI-compatible completion request (POST /v1/completions)
typedef struct {
    char prompt[MAX_PROMPT_LENGTH];
    char model[MAX_MODEL_ID_LENGTH];
    int max_tokens;
    float temperature;
    float top_p;
    int top_k;
    float frequency_penalty;
    float presence_penalty;
    char stop[MAX_STOP_SEQUENCES][MAX_STOP_LENGTH];
    int stop_count;
    bool stream;
    int seed;
    bool echo;
    int n;  // Number of completions to generate
} completion_request_t;

// Chat message structure
typedef struct {
    char role[16];      // "system", "user", "assistant"
    char content[MAX_PROMPT_LENGTH];
} chat_message_t;

// Chat completion request (POST /v1/chat/completions)
typedef struct {
    chat_message_t messages[MAX_CHAT_MESSAGES];
    int message_count;
    char model[MAX_MODEL_ID_LENGTH];
    int max_tokens;
    float temperature;
    float top_p;
    int top_k;
    float frequency_penalty;
    float presence_penalty;
    char stop[MAX_STOP_SEQUENCES][MAX_STOP_LENGTH];
    int stop_count;
    bool stream;
    int seed;
    int n;
} chat_completion_request_t;

// Embedding request (POST /v1/embeddings)
typedef struct {
    char input[MAX_PROMPT_LENGTH];
    char model[MAX_MODEL_ID_LENGTH];
} embedding_request_t;

// Tokenize request (POST /tokenize)
typedef struct {
    char content[MAX_PROMPT_LENGTH];
    bool add_special;
} tokenize_request_t;

// Detokenize request (POST /detokenize)
typedef struct {
    int tokens[MAX_TOKENS];
    int token_count;
} detokenize_request_t;

// ============================================================================
// Response Types
// ============================================================================

// Completion response choice
typedef struct {
    int index;
    char text[MAX_PROMPT_LENGTH];
    char finish_reason[16];  // "stop", "length"
} completion_choice_t;

// Chat completion response choice
typedef struct {
    int index;
    chat_message_t message;
    char finish_reason[16];
} chat_choice_t;

// Token usage statistics
typedef struct {
    int prompt_tokens;
    int completion_tokens;
    int total_tokens;
} usage_stats_t;

// Model info (for /v1/models response)
typedef struct {
    char id[MAX_MODEL_ID_LENGTH];
    char object[16];      // "model"
    char owned_by[32];
    int64_t created;
} model_info_t;

// API error response
typedef struct {
    char message[256];
    char type[32];        // "invalid_request_error", "server_error", etc.
    char param[32];       // Parameter that caused the error
    char code[32];        // Error code
} api_error_t;

// ============================================================================
// Helper Functions
// ============================================================================

// Initialize completion request with defaults
static inline void completion_request_init(completion_request_t* req) {
    req->prompt[0] = '\0';
    req->model[0] = '\0';
    req->max_tokens = DEFAULT_MAX_TOKENS;
    req->temperature = DEFAULT_TEMPERATURE;
    req->top_p = DEFAULT_TOP_P;
    req->top_k = DEFAULT_TOP_K;
    req->frequency_penalty = DEFAULT_FREQ_PENALTY;
    req->presence_penalty = DEFAULT_PRES_PENALTY;
    req->stop_count = 0;
    req->stream = false;
    req->seed = DEFAULT_SEED;
    req->echo = false;
    req->n = 1;
}

// Initialize chat completion request with defaults
static inline void chat_completion_request_init(chat_completion_request_t* req) {
    req->message_count = 0;
    req->model[0] = '\0';
    req->max_tokens = DEFAULT_MAX_TOKENS;
    req->temperature = DEFAULT_TEMPERATURE;
    req->top_p = DEFAULT_TOP_P;
    req->top_k = DEFAULT_TOP_K;
    req->frequency_penalty = DEFAULT_FREQ_PENALTY;
    req->presence_penalty = DEFAULT_PRES_PENALTY;
    req->stop_count = 0;
    req->stream = false;
    req->seed = DEFAULT_SEED;
    req->n = 1;
}

// Initialize embedding request with defaults
static inline void embedding_request_init(embedding_request_t* req) {
    req->input[0] = '\0';
    req->model[0] = '\0';
}

// Initialize tokenize request with defaults
static inline void tokenize_request_init(tokenize_request_t* req) {
    req->content[0] = '\0';
    req->add_special = true;
}

// Initialize detokenize request
static inline void detokenize_request_init(detokenize_request_t* req) {
    req->token_count = 0;
}

// Initialize API error
static inline void api_error_init(api_error_t* err) {
    err->message[0] = '\0';
    err->type[0] = '\0';
    err->param[0] = '\0';
    err->code[0] = '\0';
}

#endif // API_TYPES_H
