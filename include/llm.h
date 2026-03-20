#ifndef LLM_H
#define LLM_H

#include <stddef.h>
#include <stdint.h>

// Forward declaration
typedef struct llm_context llm_context_t;

// LLM initialization parameters
typedef struct {
    int context_size;
    int threads;
    int gpu_layers;
} llm_params_t;

// Function declarations
llm_context_t* llm_init(const uint8_t* model_data, size_t model_size, const llm_params_t* params);
void llm_free(llm_context_t* ctx);
int llm_tokenize(llm_context_t* ctx, const char* text, int* tokens, int max_tokens);
int llm_detokenize(llm_context_t* ctx, const int* tokens, int token_count, char* text, size_t text_size);
int llm_eval(llm_context_t* ctx, const int* tokens, int token_count, int n_threads);
int llm_sample(llm_context_t* ctx, int* token);

// Default parameters
llm_params_t llm_default_params(void);

#endif // LLM_H