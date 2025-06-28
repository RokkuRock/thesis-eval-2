static int __kprobes do_page_fault(unsigned long addr, unsigned int esr,
				   struct pt_regs *regs)
{
	struct task_struct *tsk;
	struct mm_struct *mm;
	int fault, sig, code;
	unsigned long vm_flags = VM_READ | VM_WRITE;
	unsigned int mm_flags = FAULT_FLAG_ALLOW_RETRY | FAULT_FLAG_KILLABLE;
	tsk = current;
	mm  = tsk->mm;
	if (interrupts_enabled(regs))
		local_irq_enable();
	if (in_atomic() || !mm)
		goto no_context;
	if (user_mode(regs))
		mm_flags |= FAULT_FLAG_USER;
	if (esr & ESR_LNX_EXEC) {
		vm_flags = VM_EXEC;
	} else if ((esr & ESR_EL1_WRITE) && !(esr & ESR_EL1_CM)) {
		vm_flags = VM_WRITE;
		mm_flags |= FAULT_FLAG_WRITE;
	}
	if (!down_read_trylock(&mm->mmap_sem)) {
		if (!user_mode(regs) && !search_exception_tables(regs->pc))
			goto no_context;
retry:
		down_read(&mm->mmap_sem);
	} else {
		might_sleep();
#ifdef CONFIG_DEBUG_VM
		if (!user_mode(regs) && !search_exception_tables(regs->pc))
			goto no_context;
#endif
	}
	fault = __do_page_fault(mm, addr, mm_flags, vm_flags, tsk);
	if ((fault & VM_FAULT_RETRY) && fatal_signal_pending(current))
		return 0;
	perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS, 1, regs, addr);
	if (mm_flags & FAULT_FLAG_ALLOW_RETRY) {
		if (fault & VM_FAULT_MAJOR) {
			tsk->maj_flt++;
			perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS_MAJ, 1, regs,
				      addr);
		} else {
			tsk->min_flt++;
			perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS_MIN, 1, regs,
				      addr);
		}
		if (fault & VM_FAULT_RETRY) {
			mm_flags &= ~FAULT_FLAG_ALLOW_RETRY;
			goto retry;
		}
	}
	up_read(&mm->mmap_sem);
	if (likely(!(fault & (VM_FAULT_ERROR | VM_FAULT_BADMAP |
			      VM_FAULT_BADACCESS))))
		return 0;
	if (!user_mode(regs))
		goto no_context;
	if (fault & VM_FAULT_OOM) {
		pagefault_out_of_memory();
		return 0;
	}
	if (fault & VM_FAULT_SIGBUS) {
		sig = SIGBUS;
		code = BUS_ADRERR;
	} else {
		sig = SIGSEGV;
		code = fault == VM_FAULT_BADACCESS ?
			SEGV_ACCERR : SEGV_MAPERR;
	}
	__do_user_fault(tsk, addr, esr, sig, code, regs);
	return 0;
no_context:
	__do_kernel_fault(mm, addr, esr, regs);
	return 0;
}