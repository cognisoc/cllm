# Change of Approach: Custom Minimal Unikernel

After attempting to build Rumprun, we've encountered build issues that would delay our progress. For our project, we'll implement a minimal custom unikernel that meets our specific needs for LLM serving rather than building on a full-featured framework like Rumprun.

This approach has several advantages:
1. **Reduced Complexity**: We only implement what we need
2. **Better Understanding**: We'll have complete knowledge of our system
3. **Faster Iteration**: Less code means easier modifications
4. **Specific Optimization**: We can optimize specifically for LLM serving workloads

We'll implement:
1. A minimal bootloader
2. Basic memory management
3. Simple I/O (console output)
4. HTTP server functionality
5. Integration with llama.cpp