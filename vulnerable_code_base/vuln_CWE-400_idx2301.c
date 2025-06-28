do_page_fault(unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
	struct task_struct *tsk;
	struct mm_struct *mm;
	int fault, sig, code;
	if (notify_page_fault(regs, fsr))
		return 0;
	tsk = current;
	mm  = tsk->mm;
	if (in_atomic() || !mm)
		goto no_context;
	if (!down_read_trylock(&mm->mmap_sem)) {
		if (!user_mode(regs) && !search_exception_tables(regs->ARM_pc))
			goto no_context;
		down_read(&mm->mmap_sem);
	} else {
		might_sleep();
#ifdef CONFIG_DEBUG_VM
		if (!user_mode(regs) &&
		    !search_exception_tables(regs->ARM_pc))
			goto no_context;
#endif
	}
	fault = __do_page_fault(mm, addr, fsr, tsk);
	up_read(&mm->mmap_sem);
	perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS, 1, 0, regs, addr);
	if (fault & VM_FAULT_MAJOR)
		perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS_MAJ, 1, 0, regs, addr);
	else if (fault & VM_FAULT_MINOR)
		perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS_MIN, 1, 0, regs, addr);
	if (likely(!(fault & (VM_FAULT_ERROR | VM_FAULT_BADMAP | VM_FAULT_BADACCESS))))
		return 0;
	if (fault & VM_FAULT_OOM) {
		pagefault_out_of_memory();
		return 0;
	}
	if (!user_mode(regs))
		goto no_context;
	if (fault & VM_FAULT_SIGBUS) {
		sig = SIGBUS;
		code = BUS_ADRERR;
	} else {
		sig = SIGSEGV;
		code = fault == VM_FAULT_BADACCESS ?
			SEGV_ACCERR : SEGV_MAPERR;
	}
	__do_user_fault(tsk, addr, fsr, sig, code, regs);
	return 0;
no_context:
	__do_kernel_fault(mm, addr, fsr, regs);
	return 0;
}