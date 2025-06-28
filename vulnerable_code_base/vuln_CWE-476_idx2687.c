static ram_addr_t find_ram_offset(struct uc_struct *uc, ram_addr_t size)
{
    RAMBlock *block, *next_block;
    ram_addr_t offset = RAM_ADDR_MAX, mingap = RAM_ADDR_MAX;
    assert(size != 0);  
    if (QLIST_EMPTY(&uc->ram_list.blocks)) {
        return 0;
    }
    RAMBLOCK_FOREACH(block) {
        ram_addr_t candidate, next = RAM_ADDR_MAX;
        candidate = block->offset + block->max_length;
        candidate = ROUND_UP(candidate, BITS_PER_LONG << TARGET_PAGE_BITS);
        RAMBLOCK_FOREACH(next_block) {
            if (next_block->offset >= candidate) {
                next = MIN(next, next_block->offset);
            }
        }
        if (next - candidate >= size && next - candidate < mingap) {
            offset = candidate;
            mingap = next - candidate;
        }
    }
    if (offset == RAM_ADDR_MAX) {
        fprintf(stderr, "Failed to find gap of requested size: %" PRIu64 "\n",
                (uint64_t)size);
        abort();
    }
    return offset;
}