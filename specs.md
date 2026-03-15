The goal of this project is do the following in phases
1. Have a working C unikernel in qemu targeting x86_64
2. Compile llama.cpp into unikernel to serve models over http, there will be a json based configuration provided for the unikernel build
3. Add GPU support to the unikernel, we want to support all the backends (starting with CUDA) as done by llama.cpp 
4. Port optimisations from vllm into llama.cpp to accelerate serving of transformer models

Each phase is important and needs to be done correctly. We want to build out the right code abstractions and add detailed comments

The root folder is where the entire project should reside, have a docs folder for all working documents.
