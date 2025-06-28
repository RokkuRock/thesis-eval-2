int __kprobes do_page_fault(struct pt_regs *regs, unsigned long address,
			    unsigned long error_code)
{
	struct vm_area_struct * vma;
	struct mm_struct *mm = current->mm;
	siginfo_t info;
	int code = SEGV_MAPERR;
	int is_write = 0, ret;
	int trap = TRAP(regs);
 	int is_exec = trap == 0x400;
#if !(defined(CONFIG_4xx) || defined(CONFIG_BOOKE))
	if (trap == 0x400)
		error_code &= 0x48200000;
	else
		is_write = error_code & DSISR_ISSTORE;
#else
	is_write = error_code & ESR_DST;
#endif  
	if (notify_page_fault(regs))
		return 0;
	if (unlikely(debugger_fault_handler(regs)))
		return 0;
	if (!user_mode(regs) && (address >= TASK_SIZE))
		return SIGSEGV;
#if !(defined(CONFIG_4xx) || defined(CONFIG_BOOKE) || \
			     defined(CONFIG_PPC_BOOK3S_64))
  	if (error_code & DSISR_DABRMATCH) {
		do_dabr(regs, address, error_code);
		return 0;
	}
#endif
	if (in_atomic() || mm == NULL) {
		if (!user_mode(regs))
			return SIGSEGV;
		printk(KERN_EMERG "Page fault in user mode with "
		       "in_atomic() = %d mm = %p\n", in_atomic(), mm);
		printk(KERN_EMERG "NIP = %lx  MSR = %lx\n",
		       regs->nip, regs->msr);
		die("Weird page fault", regs, SIGSEGV);
	}
	perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS, 1, 0, regs, address);
	if (!down_read_trylock(&mm->mmap_sem)) {
		if (!user_mode(regs) && !search_exception_tables(regs->nip))
			goto bad_area_nosemaphore;
		down_read(&mm->mmap_sem);
	}
	vma = find_vma(mm, address);
	if (!vma)
		goto bad_area;
	if (vma->vm_start <= address)
		goto good_area;
	if (!(vma->vm_flags & VM_GROWSDOWN))
		goto bad_area;
	if (address + 0x100000 < vma->vm_end) {
		struct pt_regs *uregs = current->thread.regs;
		if (uregs == NULL)
			goto bad_area;
		if (address + 2048 < uregs->gpr[1]
		    && (!user_mode(regs) || !store_updates_sp(regs)))
			goto bad_area;
	}
	if (expand_stack(vma, address))
		goto bad_area;
good_area:
	code = SEGV_ACCERR;
#if defined(CONFIG_6xx)
	if (error_code & 0x95700000)
		goto bad_area;
#endif  
#if defined(CONFIG_8xx)
	if (error_code & 0x40000000)  
		_tlbil_va(address, 0, 0, 0);
	if (error_code & 0x10000000)
		goto bad_area;
#endif  
	if (is_exec) {
#ifdef CONFIG_PPC_STD_MMU
		if (error_code & DSISR_PROTFAULT)
			goto bad_area;
#endif  
		if (!(vma->vm_flags & VM_EXEC) &&
		    (cpu_has_feature(CPU_FTR_NOEXECUTE) ||
		     !(vma->vm_flags & (VM_READ | VM_WRITE))))
			goto bad_area;
	} else if (is_write) {
		if (!(vma->vm_flags & VM_WRITE))
			goto bad_area;
	} else {
		if (error_code & 0x08000000)
			goto bad_area;
		if (!(vma->vm_flags & (VM_READ | VM_EXEC | VM_WRITE)))
			goto bad_area;
	}
	ret = handle_mm_fault(mm, vma, address, is_write ? FAULT_FLAG_WRITE : 0);
	if (unlikely(ret & VM_FAULT_ERROR)) {
		if (ret & VM_FAULT_OOM)
			goto out_of_memory;
		else if (ret & VM_FAULT_SIGBUS)
			goto do_sigbus;
		BUG();
	}
	if (ret & VM_FAULT_MAJOR) {
		current->maj_flt++;
		perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS_MAJ, 1, 0,
				     regs, address);
#ifdef CONFIG_PPC_SMLPAR
		if (firmware_has_feature(FW_FEATURE_CMO)) {
			preempt_disable();
			get_lppaca()->page_ins += (1 << PAGE_FACTOR);
			preempt_enable();
		}
#endif
	} else {
		current->min_flt++;
		perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS_MIN, 1, 0,
				     regs, address);
	}
	up_read(&mm->mmap_sem);
	return 0;
bad_area:
	up_read(&mm->mmap_sem);
bad_area_nosemaphore:
	if (user_mode(regs)) {
		_exception(SIGSEGV, regs, code, address);
		return 0;
	}
	if (is_exec && (error_code & DSISR_PROTFAULT)
	    && printk_ratelimit())
		printk(KERN_CRIT "kernel tried to execute NX-protected"
		       " page (%lx) - exploit attempt? (uid: %d)\n",
		       address, current_uid());
	return SIGSEGV;
out_of_memory:
	up_read(&mm->mmap_sem);
	if (!user_mode(regs))
		return SIGKILL;
	pagefault_out_of_memory();
	return 0;
do_sigbus:
	up_read(&mm->mmap_sem);
	if (user_mode(regs)) {
		info.si_signo = SIGBUS;
		info.si_errno = 0;
		info.si_code = BUS_ADRERR;
		info.si_addr = (void __user *)address;
		force_sig_info(SIGBUS, &info, current);
		return 0;
	}
	return SIGBUS;
}