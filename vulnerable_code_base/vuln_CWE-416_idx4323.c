unsigned long insn_get_seg_base(struct pt_regs *regs, int seg_reg_idx)
{
	struct desc_struct *desc;
	short sel;
	sel = get_segment_selector(regs, seg_reg_idx);
	if (sel < 0)
		return -1L;
	if (v8086_mode(regs))
		return (unsigned long)(sel << 4);
	if (user_64bit_mode(regs)) {
		unsigned long base;
		if (seg_reg_idx == INAT_SEG_REG_FS)
			rdmsrl(MSR_FS_BASE, base);
		else if (seg_reg_idx == INAT_SEG_REG_GS)
			rdmsrl(MSR_KERNEL_GS_BASE, base);
		else
			base = 0;
		return base;
	}
	if (!sel)
		return -1L;
	desc = get_desc(sel);
	if (!desc)
		return -1L;
	return get_desc_base(desc);
}