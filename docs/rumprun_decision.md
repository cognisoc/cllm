# Decision to Use Rumprun

After researching unikernel implementations, we've decided to build our LLM serving unikernel on top of Rumprun for the following reasons:

1. **Existing Infrastructure**: Rumprun provides a solid foundation with existing kernel services like networking and memory management
2. **POSIX Compatibility**: It offers a POSIX-like interface which will make it easier to integrate llama.cpp
3. **QEMU Support**: Rumprun supports QEMU as a target, which matches our requirements
4. **Faster Development**: Building on Rumprun will allow us to focus on our core value proposition (LLM serving) rather than reimplementing basic kernel services
5. **Community Support**: Using an established project means we can benefit from existing documentation and community knowledge

This approach aligns with our project goals while reducing development time and potential bugs.