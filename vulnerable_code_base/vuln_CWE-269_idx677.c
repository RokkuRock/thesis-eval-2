static int __poke_user(struct task_struct *child, addr_t addr, addr_t data)
{
	struct user *dummy = NULL;
	addr_t offset;
	if (addr < (addr_t) &dummy->regs.acrs) {
		if (addr == (addr_t) &dummy->regs.psw.mask) {
			unsigned long mask = PSW_MASK_USER;
			mask |= is_ri_task(child) ? PSW_MASK_RI : 0;
			if ((data & ~mask) != PSW_USER_BITS)
				return -EINVAL;
			if ((data & PSW_MASK_EA) && !(data & PSW_MASK_BA))
				return -EINVAL;
		}
		*(addr_t *)((addr_t) &task_pt_regs(child)->psw + addr) = data;
	} else if (addr < (addr_t) (&dummy->regs.orig_gpr2)) {
		offset = addr - (addr_t) &dummy->regs.acrs;
#ifdef CONFIG_64BIT
		if (addr == (addr_t) &dummy->regs.acrs[15])
			child->thread.acrs[15] = (unsigned int) (data >> 32);
		else
#endif
		*(addr_t *)((addr_t) &child->thread.acrs + offset) = data;
	} else if (addr == (addr_t) &dummy->regs.orig_gpr2) {
		task_pt_regs(child)->orig_gpr2 = data;
	} else if (addr < (addr_t) &dummy->regs.fp_regs) {
		return 0;
	} else if (addr < (addr_t) (&dummy->regs.fp_regs + 1)) {
		if (addr == (addr_t) &dummy->regs.fp_regs.fpc)
			if ((unsigned int) data != 0 ||
			    test_fp_ctl(data >> (BITS_PER_LONG - 32)))
				return -EINVAL;
		offset = addr - (addr_t) &dummy->regs.fp_regs;
		*(addr_t *)((addr_t) &child->thread.fp_regs + offset) = data;
	} else if (addr < (addr_t) (&dummy->regs.per_info + 1)) {
		addr -= (addr_t) &dummy->regs.per_info;
		__poke_user_per(child, addr, data);
	}
	return 0;
}