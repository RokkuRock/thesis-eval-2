asmlinkage void __kprobes do_page_fault(struct pt_regs *regs,
					unsigned long writeaccess,
					unsigned long address)
{
	unsigned long vec;
	struct task_struct *tsk;
	struct mm_struct *mm;
	struct vm_area_struct * vma;
	int si_code;
	int fault;
	siginfo_t info;
	tsk = current;
	mm = tsk->mm;
	si_code = SEGV_MAPERR;
	vec = lookup_exception_vector();
	if (unlikely(fault_in_kernel_space(address))) {
		if (vmalloc_fault(address) >= 0)
			return;
		if (notify_page_fault(regs, vec))
			return;
		goto bad_area_nosemaphore;
	}
	if (unlikely(notify_page_fault(regs, vec)))
		return;
	if ((regs->sr & SR_IMASK) != SR_IMASK)
		local_irq_enable();
	perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS, 1, 0, regs, address);
	if (in_atomic() || !mm)
		goto no_context;
	down_read(&mm->mmap_sem);
	vma = find_vma(mm, address);
	if (!vma)
		goto bad_area;
	if (vma->vm_start <= address)
		goto good_area;
	if (!(vma->vm_flags & VM_GROWSDOWN))
		goto bad_area;
	if (expand_stack(vma, address))
		goto bad_area;
good_area:
	si_code = SEGV_ACCERR;
	if (writeaccess) {
		if (!(vma->vm_flags & VM_WRITE))
			goto bad_area;
	} else {
		if (!(vma->vm_flags & (VM_READ | VM_EXEC | VM_WRITE)))
			goto bad_area;
	}
	fault = handle_mm_fault(mm, vma, address, writeaccess ? FAULT_FLAG_WRITE : 0);
	if (unlikely(fault & VM_FAULT_ERROR)) {
		if (fault & VM_FAULT_OOM)
			goto out_of_memory;
		else if (fault & VM_FAULT_SIGBUS)
			goto do_sigbus;
		BUG();
	}
	if (fault & VM_FAULT_MAJOR) {
		tsk->maj_flt++;
		perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS_MAJ, 1, 0,
				     regs, address);
	} else {
		tsk->min_flt++;
		perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS_MIN, 1, 0,
				     regs, address);
	}
	up_read(&mm->mmap_sem);
	return;
bad_area:
	up_read(&mm->mmap_sem);
bad_area_nosemaphore:
	if (user_mode(regs)) {
		info.si_signo = SIGSEGV;
		info.si_errno = 0;
		info.si_code = si_code;
		info.si_addr = (void *) address;
		force_sig_info(SIGSEGV, &info, tsk);
		return;
	}
no_context:
	if (fixup_exception(regs))
		return;
	if (handle_trapped_io(regs, address))
		return;
	bust_spinlocks(1);
	if (oops_may_print()) {
		unsigned long page;
		if (address < PAGE_SIZE)
			printk(KERN_ALERT "Unable to handle kernel NULL "
					  "pointer dereference");
		else
			printk(KERN_ALERT "Unable to handle kernel paging "
					  "request");
		printk(" at virtual address %08lx\n", address);
		printk(KERN_ALERT "pc = %08lx\n", regs->pc);
		page = (unsigned long)get_TTB();
		if (page) {
			page = ((__typeof__(page) *)page)[address >> PGDIR_SHIFT];
			printk(KERN_ALERT "*pde = %08lx\n", page);
			if (page & _PAGE_PRESENT) {
				page &= PAGE_MASK;
				address &= 0x003ff000;
				page = ((__typeof__(page) *)
						__va(page))[address >>
							    PAGE_SHIFT];
				printk(KERN_ALERT "*pte = %08lx\n", page);
			}
		}
	}
	die("Oops", regs, writeaccess);
	bust_spinlocks(0);
	do_exit(SIGKILL);
out_of_memory:
	up_read(&mm->mmap_sem);
	if (!user_mode(regs))
		goto no_context;
	pagefault_out_of_memory();
	return;
do_sigbus:
	up_read(&mm->mmap_sem);
	info.si_signo = SIGBUS;
	info.si_errno = 0;
	info.si_code = BUS_ADRERR;
	info.si_addr = (void *)address;
	force_sig_info(SIGBUS, &info, tsk);
	if (!user_mode(regs))
		goto no_context;
}