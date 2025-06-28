static void *getMcontextEip(ucontext_t *uc) {
#define NOT_SUPPORTED() do {\
    UNUSED(uc);\
    return NULL;\
} while(0)
#if defined(__APPLE__) && !defined(MAC_OS_X_VERSION_10_6)
    #if defined(__x86_64__)
    return (void*) uc->uc_mcontext->__ss.__rip;
    #elif defined(__i386__)
    return (void*) uc->uc_mcontext->__ss.__eip;
    #else
    return (void*) uc->uc_mcontext->__ss.__srr0;
    #endif
#elif defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_6)
    #if defined(_STRUCT_X86_THREAD_STATE64) && !defined(__i386__)
    return (void*) uc->uc_mcontext->__ss.__rip;
    #elif defined(__i386__)
    return (void*) uc->uc_mcontext->__ss.__eip;
    #else
    return (void*) arm_thread_state64_get_pc(uc->uc_mcontext->__ss);
    #endif
#elif defined(__linux__)
    #if defined(__i386__) || ((defined(__X86_64__) || defined(__x86_64__)) && defined(__ILP32__))
    return (void*) uc->uc_mcontext.gregs[14];  
    #elif defined(__X86_64__) || defined(__x86_64__)
    return (void*) uc->uc_mcontext.gregs[16];  
    #elif defined(__ia64__)  
    return (void*) uc->uc_mcontext.sc_ip;
    #elif defined(__arm__)  
    return (void*) uc->uc_mcontext.arm_pc;
    #elif defined(__aarch64__)  
    return (void*) uc->uc_mcontext.pc;
    #else
    NOT_SUPPORTED();
    #endif
#elif defined(__FreeBSD__)
    #if defined(__i386__)
    return (void*) uc->uc_mcontext.mc_eip;
    #elif defined(__x86_64__)
    return (void*) uc->uc_mcontext.mc_rip;
    #else
    NOT_SUPPORTED();
    #endif
#elif defined(__OpenBSD__)
    #if defined(__i386__)
    return (void*) uc->sc_eip;
    #elif defined(__x86_64__)
    return (void*) uc->sc_rip;
    #else
    NOT_SUPPORTED();
    #endif
#elif defined(__NetBSD__)
    #if defined(__i386__)
    return (void*) uc->uc_mcontext.__gregs[_REG_EIP];
    #elif defined(__x86_64__)
    return (void*) uc->uc_mcontext.__gregs[_REG_RIP];
    #else
    NOT_SUPPORTED();
    #endif
#elif defined(__DragonFly__)
    return (void*) uc->uc_mcontext.mc_rip;
#else
    NOT_SUPPORTED();
#endif
#undef NOT_SUPPORTED
}