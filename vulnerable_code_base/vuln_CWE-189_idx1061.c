void* leak_malloc(size_t bytes)
{
    void* base = dlmalloc(bytes + sizeof(AllocationEntry));
    if (base != NULL) {
        pthread_mutex_lock(&gAllocationsMutex);
            intptr_t backtrace[BACKTRACE_SIZE];
            size_t numEntries = get_backtrace(backtrace, BACKTRACE_SIZE);
            AllocationEntry* header = (AllocationEntry*)base;
            header->entry = record_backtrace(backtrace, numEntries, bytes);
            header->guard = GUARD;
            base = (AllocationEntry*)base + 1;
        pthread_mutex_unlock(&gAllocationsMutex);
    }
    return base;
}