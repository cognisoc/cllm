/*
 * llm.cpp - Optional C++ bridge to upstream llama.cpp for real inference.
 *
 * This file is compiled only when building with llama.cpp support. It exposes
 * a small C-compatible wrapper around the upstream llama.cpp C API and uses
 * memory-loaded model data instead of filesystem I/O.
 */

// This file is compiled as C++ but must not pull in the host libc headers
// that conflict with the unikernel's string.h replacements.
#define __CLLM_LLM_CPP__

#include "llm.h"
#include "kernel.h"
#include "string.h"

// Conditionally include upstream llama.cpp headers. When USE_LLAMA_CPP is not
// defined, this file is not compiled at all; the C-only stub in llm.c is used.
#include "llama.h"
#include "ggml.h"

// Host-provided embedded model data. The build system generates a C array with
// this symbol when a real model is embedded.
extern const uint8_t embedded_gguf_model[];
extern const size_t embedded_gguf_model_size;

struct llm_context {
    llama_model* model;
    llama_context* ctx;
    llama_sampler* sampler;
    llama_batch batch;
    int n_vocab;
    int n_batch;
};

// C++ runtime / OS replacement stubs used by llama.cpp when it is linked into
// the unikernel. These are weak so the real kernel versions can override them.
extern "C" {

// Minimal new/delete operators that use the unikernel allocator.
void* operator new(size_t size) {
    return malloc(size);
}
void* operator new[](size_t size) {
    return malloc(size);
}
void operator delete(void* ptr) noexcept {
    free(ptr);
}
void operator delete[](void* ptr) noexcept {
    free(ptr);
}
void operator delete(void* ptr, size_t) noexcept {
    free(ptr);
}
void operator delete[](void* ptr, size_t) noexcept {
    free(ptr);
}

// Abort handler.
extern "C" void __cxa_pure_virtual(void) {
    serial_write("LLM: pure virtual call\n");
    while (1) {
        __asm__("hlt");
    }
}

} // extern "C"

llm_params_t llm_default_params(void) {
    llm_params_t params = {
        .context_size = 512,
        .threads = 1,
        .gpu_layers = 0
    };
    return params;
}

llm_context_t* llm_init(const uint8_t* model_data, size_t model_size, const llm_params_t* params) {
    (void)model_data;
    (void)model_size;

    int n_threads = params ? params->threads : 1;
    int n_ctx     = params ? params->context_size : 512;
    int n_gpu_layers = params ? params->gpu_layers : 0;

    const uint8_t* data = embedded_gguf_model;
    size_t size = embedded_gguf_model_size;
    if (!data || size == 0) {
        serial_write("LLM: No embedded GGUF model available\n");
        return NULL;
    }

    llm_context_t* llm = (llm_context_t*)malloc(sizeof(llm_context_t));
    if (!llm) return NULL;
    memset(llm, 0, sizeof(*llm));

    llama_model_params mparams = llama_model_default_params();
    mparams.n_gpu_layers = n_gpu_layers;
    // Force CPU-only backend registration; disable dynamic backend loading.
    mparams.use_mlock = false;
    mparams.use_mmap  = false;

    // llama.cpp b4499 only exposes file-based model loading. We therefore need a
    // memory-backed "file". The unikernel provides a stub fopen/fread that reads
    // from the embedded model. Pass a sentinel path that llama.cpp will open.
    // A build-time tool can alternatively generate a temporary file in the initrd.
    (void)data;
    (void)size;
    llama_model* model = llama_model_load_from_file("/model.gguf", mparams);
    if (!model) {
        serial_write("LLM: Failed to load model from embedded data\n");
        free(llm);
        return NULL;
    }

    llama_context_params cparams = llama_context_default_params();
    cparams.n_ctx = n_ctx;
    cparams.n_batch = n_ctx;
    cparams.n_threads = n_threads;
    cparams.n_threads_batch = n_threads;
    cparams.embeddings = false;

    llama_context* ctx = llama_init_from_model(model, cparams);
    if (!ctx) {
        serial_write("LLM: Failed to create llama context\n");
        llama_model_free(model);
        free(llm);
        return NULL;
    }

    llm->model = model;
    llm->ctx = ctx;
    llm->n_vocab = llama_vocab_n_tokens(llama_model_get_vocab(model));
    llm->n_batch = cparams.n_batch;

    // Greedy sampler for deterministic output.
    llm->sampler = llama_sampler_chain_init(llama_sampler_chain_default_params());
    llama_sampler_chain_add(llm->sampler, llama_sampler_init_greedy());

    llm->batch = llama_batch_init(llm->n_batch, 0, 1);

    char buf[64];
    snprintf(buf, sizeof(buf), "LLM: Model loaded, vocab=%d\n", llm->n_vocab);
    serial_write(buf);
    return llm;
}

void llm_free(llm_context_t* llm) {
    if (!llm) return;
    if (llm->sampler) {
        llama_sampler_free(llm->sampler);
    }
    if (llm->ctx) {
        llama_free(llm->ctx);
    }
    if (llm->model) {
        llama_model_free(llm->model);
    }
    llama_batch_free(llm->batch);
    free(llm);
}

int llm_tokenize(llm_context_t* llm, const char* text, int* tokens, int max_tokens) {
    if (!llm || !text || !tokens || max_tokens <= 0) return -1;
    return llama_tokenize(llama_model_get_vocab(llm->model), text, (int32_t)strlen(text),
                          tokens, max_tokens, true, false);
}

int llm_detokenize(llm_context_t* llm, const int* tokens, int token_count, char* text, size_t text_size) {
    if (!llm || !tokens || token_count <= 0 || !text || text_size == 0) return -1;
    const llama_vocab* vocab = llama_model_get_vocab(llm->model);
    size_t offset = 0;
    for (int i = 0; i < token_count && offset + 1 < text_size; i++) {
        const char* piece = llama_vocab_get_text(vocab, tokens[i]);
        size_t len = strlen(piece);
        if (offset + len >= text_size) {
            len = text_size - offset - 1;
        }
        memcpy(text + offset, piece, len);
        offset += len;
    }
    text[offset] = '\0';
    return (int)offset;
}

int llm_eval(llm_context_t* llm, const int* tokens, int token_count, int n_threads) {
    if (!llm || !tokens || token_count <= 0) return -1;
    (void)n_threads;

    llm->batch.n_tokens = token_count;
    for (int i = 0; i < token_count; i++) {
        llm->batch.token[i] = tokens[i];
        llm->batch.pos[i] = i;
        llm->batch.n_seq_id[i] = 1;
        llm->batch.seq_id[i][0] = 0;
        llm->batch.logits[i] = false;
    }
    llm->batch.logits[token_count - 1] = true;

    if (llama_decode(llm->ctx, llm->batch) != 0) {
        serial_write("LLM: llama_decode failed\n");
        return -1;
    }
    return 0;
}

int llm_generate(llm_context_t* llm, const char* prompt, int max_tokens,
                   char* output, size_t output_size) {
    if (!llm || !prompt || !output || output_size == 0) return -1;

    int prompt_tokens[512];
    int n_prompt = llm_tokenize(llm, prompt, prompt_tokens, 512);
    if (n_prompt < 0) return -1;

    llama_kv_cache_clear(llm->ctx);

    if (llm_eval(llm, prompt_tokens, n_prompt, 1) != 0) return -1;

    int prev_tokens = n_prompt;
    size_t out_len = 0;
    output[0] = '\0';

    for (int i = 0; i < max_tokens; i++) {
        int next;
        if (llm_sample(llm, &next) != 0) break;

        const llama_vocab* vocab = llama_model_get_vocab(llm->model);
        if (llama_vocab_is_eog(vocab, next)) break;

        const char* piece = llama_vocab_get_text(vocab, next);
        size_t plen = strlen(piece);
        if (out_len + plen >= output_size) break;
        memcpy(output + out_len, piece, plen);
        out_len += plen;
        output[out_len] = '\0';

        if (llm_eval(llm, &next, 1, 1) != 0) break;
        prev_tokens++;
    }

    return (int)out_len;
}

int llm_sample(llm_context_t* llm, int* token) {
    if (!llm || !token) return -1;
    int n_tokens = llama_get_kv_cache_used_cells(llm->ctx);
    if (n_tokens <= 0) return -1;
    int32_t last = n_tokens - 1;
    *token = llama_sampler_sample(llm->sampler, llm->ctx, last);
    return 0;
}
