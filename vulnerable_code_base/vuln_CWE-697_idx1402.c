static void uc_invalidate_tb(struct uc_struct *uc, uint64_t start_addr, size_t len) 
{
    tb_page_addr_t start, end;
    start = get_page_addr_code(uc->cpu->env_ptr, start_addr) & (target_ulong)(-1);
    end = (start + len) & (target_ulong)(-1);
    if (start > end) {
        return;
    }
    tb_invalidate_phys_range(uc, start, end);
}