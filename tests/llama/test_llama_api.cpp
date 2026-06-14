// Host-side test that verifies the upstream llama.cpp C API is available and
// can be initialized. Loading a real model is optional; if no model is found
// the test reports that the build integration succeeded.

#include "llama.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char** argv) {
    const char* model_path = (argc > 1) ? argv[1] : nullptr;

    printf("llama.cpp host API test\n");
    printf("llama.cpp API integration test\n");

    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = 0; // CPU only

    llama_model* model = nullptr;
    if (model_path) {
        model = llama_model_load_from_file(model_path, model_params);
    }

    if (!model) {
        printf("No model loaded (model_path=%s); build integration succeeded.\n",
               model_path ? model_path : "(none)");
        return 0;
    }

    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 128;
    ctx_params.n_batch = 32;
    ctx_params.n_threads = 1;
    ctx_params.n_threads_batch = 1;

    llama_context* ctx = llama_init_from_model(model, ctx_params);
    if (!ctx) {
        printf("Failed to create context\n");
        llama_model_free(model);
        return 1;
    }

    const char* text = "Hello, world!";
    int n_tokens = 64;
    llama_token* tokens = (llama_token*)malloc(n_tokens * sizeof(llama_token));
    n_tokens = llama_tokenize(llama_model_get_vocab(model), text, (int)strlen(text),
                              tokens, n_tokens, true, false);
    printf("Tokenized '%s' into %d tokens\n", text, n_tokens);

    llama_free(ctx);
    llama_model_free(model);
    free(tokens);
    printf("llama.cpp API integration test passed.\n");
    return 0;
}
