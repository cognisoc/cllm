/*
 * api_v1.h - llama.cpp-compatible API v1 handlers
 */

#ifndef API_V1_H
#define API_V1_H

#include "network.h"
#include "llm.h"

// Initialize API context with LLM context
int api_v1_init(llm_context_t* llm_ctx);

// Endpoint handlers
int handle_v1_completions(http_request_t* request, http_response_t* response);
int handle_v1_chat_completions(http_request_t* request, http_response_t* response);
int handle_v1_embeddings(http_request_t* request, http_response_t* response);
int handle_v1_models(http_request_t* request, http_response_t* response);
int handle_tokenize(http_request_t* request, http_response_t* response);
int handle_detokenize(http_request_t* request, http_response_t* response);

#endif // API_V1_H
