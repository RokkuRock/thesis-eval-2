static inline void tlb_end_vma(struct mmu_gather *tlb, struct vm_area_struct *vma)
{
	if (tlb->fullmm || IS_ENABLED(CONFIG_MMU_GATHER_MERGE_VMAS))
		return;
	tlb_flush_mmu_tlbonly(tlb);
}