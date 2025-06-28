static struct desc_struct *get_desc(unsigned short sel)
{
	struct desc_ptr gdt_desc = {0, 0};
	unsigned long desc_base;
#ifdef CONFIG_MODIFY_LDT_SYSCALL
	if ((sel & SEGMENT_TI_MASK) == SEGMENT_LDT) {
		struct desc_struct *desc = NULL;
		struct ldt_struct *ldt;
		sel >>= 3;
		mutex_lock(&current->active_mm->context.lock);
		ldt = current->active_mm->context.ldt;
		if (ldt && sel < ldt->nr_entries)
			desc = &ldt->entries[sel];
		mutex_unlock(&current->active_mm->context.lock);
		return desc;
	}
#endif
	native_store_gdt(&gdt_desc);
	desc_base = sel & ~(SEGMENT_RPL_MASK | SEGMENT_TI_MASK);
	if (desc_base > gdt_desc.size)
		return NULL;
	return (struct desc_struct *)(gdt_desc.address + desc_base);
}