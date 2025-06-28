int do_mathemu(struct pt_regs *regs, struct task_struct *fpt)
{
	int i;
	int retcode = 0;                                
	unsigned long insn;
	perf_sw_event(PERF_COUNT_SW_EMULATION_FAULTS, 1, 0, regs, 0);
#ifdef DEBUG_MATHEMU
	printk("In do_mathemu()... pc is %08lx\n", regs->pc);
	printk("fpqdepth is %ld\n", fpt->thread.fpqdepth);
	for (i = 0; i < fpt->thread.fpqdepth; i++)
		printk("%d: %08lx at %08lx\n", i, fpt->thread.fpqueue[i].insn,
		       (unsigned long)fpt->thread.fpqueue[i].insn_addr);
#endif
	if (fpt->thread.fpqdepth == 0) {                    
#ifdef DEBUG_MATHEMU
		printk("precise trap at %08lx\n", regs->pc);
#endif
		if (!get_user(insn, (u32 __user *) regs->pc)) {
			retcode = do_one_mathemu(insn, &fpt->thread.fsr, fpt->thread.float_regs);
			if (retcode) {
				regs->pc = regs->npc;
				regs->npc += 4;
			}
		}
		return retcode;
	}
	for (i = 0; i < fpt->thread.fpqdepth; i++) {
		retcode = do_one_mathemu(fpt->thread.fpqueue[i].insn, &(fpt->thread.fsr), fpt->thread.float_regs);
		if (!retcode)                                
			break;
	}
	if (retcode)
		fpt->thread.fsr &= ~(0x3000 | FSR_CEXC_MASK);
	else
		fpt->thread.fsr &= ~0x3000;
	fpt->thread.fpqdepth = 0;
	return retcode;
}