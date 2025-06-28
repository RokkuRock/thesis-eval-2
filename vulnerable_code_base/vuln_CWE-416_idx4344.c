int insn_get_code_seg_params(struct pt_regs *regs)
{
	struct desc_struct *desc;
	short sel;
	if (v8086_mode(regs))
		return INSN_CODE_SEG_PARAMS(2, 2);
	sel = get_segment_selector(regs, INAT_SEG_REG_CS);
	if (sel < 0)
		return sel;
	desc = get_desc(sel);
	if (!desc)
		return -EINVAL;
	if (!(desc->type & BIT(3)))
		return -EINVAL;
	switch ((desc->l << 1) | desc->d) {
	case 0:  
		return INSN_CODE_SEG_PARAMS(2, 2);
	case 1:  
		return INSN_CODE_SEG_PARAMS(4, 4);
	case 2:  
		return INSN_CODE_SEG_PARAMS(4, 8);
	case 3:  
	default:
		return -EINVAL;
	}
}