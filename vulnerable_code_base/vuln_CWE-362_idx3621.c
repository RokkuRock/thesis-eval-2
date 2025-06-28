static unsigned long get_seg_limit(struct pt_regs *regs, int seg_reg_idx)
{
	struct desc_struct *desc;
	unsigned long limit;
	short sel;
	sel = get_segment_selector(regs, seg_reg_idx);
	if (sel < 0)
		return 0;
	if (user_64bit_mode(regs) || v8086_mode(regs))
		return -1L;
	if (!sel)
		return 0;
	desc = get_desc(sel);
	if (!desc)
		return 0;
	limit = get_desc_limit(desc);
	if (desc->g)
		limit = (limit << 12) + 0xfff;
	return limit;
}