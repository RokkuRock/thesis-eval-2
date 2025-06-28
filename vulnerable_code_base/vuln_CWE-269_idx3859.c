static int __poke_user_compat(struct task_struct *child,
			      addr_t addr, addr_t data)
{
	struct compat_user *dummy32 = NULL;
	__u32 tmp = (__u32) data;
	addr_t offset;
	if (addr < (addr_t) &dummy32->regs.acrs) {
		struct pt_regs *regs = task_pt_regs(child);
		if (addr == (addr_t) &dummy32->regs.psw.mask) {
			__u32 mask = PSW32_MASK_USER;
			mask |= is_ri_task(child) ? PSW32_MASK_RI : 0;
			if ((tmp & ~mask) != PSW32_USER_BITS)
				return -EINVAL;
			regs->psw.mask = (regs->psw.mask & ~PSW_MASK_USER) |
				(regs->psw.mask & PSW_MASK_BA) |
				(__u64)(tmp & mask) << 32;
		} else if (addr == (addr_t) &dummy32->regs.psw.addr) {
			regs->psw.addr = (__u64) tmp & PSW32_ADDR_INSN;
			regs->psw.mask = (regs->psw.mask & ~PSW_MASK_BA) |
				(__u64)(tmp & PSW32_ADDR_AMODE);
		} else {
			*(__u32*)((addr_t) &regs->psw + addr*2 + 4) = tmp;
		}
	} else if (addr < (addr_t) (&dummy32->regs.orig_gpr2)) {
		offset = addr - (addr_t) &dummy32->regs.acrs;
		*(__u32*)((addr_t) &child->thread.acrs + offset) = tmp;
	} else if (addr == (addr_t) (&dummy32->regs.orig_gpr2)) {
		*(__u32*)((addr_t) &task_pt_regs(child)->orig_gpr2 + 4) = tmp;
	} else if (addr < (addr_t) &dummy32->regs.fp_regs) {
		return 0;
	} else if (addr < (addr_t) (&dummy32->regs.fp_regs + 1)) {
		if (addr == (addr_t) &dummy32->regs.fp_regs.fpc &&
		    test_fp_ctl(tmp))
			return -EINVAL;
	        offset = addr - (addr_t) &dummy32->regs.fp_regs;
		*(__u32 *)((addr_t) &child->thread.fp_regs + offset) = tmp;
	} else if (addr < (addr_t) (&dummy32->regs.per_info + 1)) {
		addr -= (addr_t) &dummy32->regs.per_info;
		__poke_user_per_compat(child, addr, data);
	}
	return 0;
}