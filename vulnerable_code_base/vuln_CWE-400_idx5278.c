asmlinkage void __kprobes do_sparc64_fault(struct pt_regs *regs)
{
	struct mm_struct *mm = current->mm;
	struct vm_area_struct *vma;
	unsigned int insn = 0;
	int si_code, fault_code, fault;
	unsigned long address, mm_rss;
	fault_code = get_thread_fault_code();
	if (notify_page_fault(regs))
		return;
	si_code = SEGV_MAPERR;
	address = current_thread_info()->fault_address;
	if ((fault_code & FAULT_CODE_ITLB) &&
	    (fault_code & FAULT_CODE_DTLB))
		BUG();
	if (test_thread_flag(TIF_32BIT)) {
		if (!(regs->tstate & TSTATE_PRIV)) {
			if (unlikely((regs->tpc >> 32) != 0)) {
				bogus_32bit_fault_tpc(regs);
				goto intr_or_no_mm;
			}
		}
		if (unlikely((address >> 32) != 0)) {
			bogus_32bit_fault_address(regs, address);
			goto intr_or_no_mm;
		}
	}
	if (regs->tstate & TSTATE_PRIV) {
		unsigned long tpc = regs->tpc;
		if ((tpc >= KERNBASE && tpc < (unsigned long) __init_end) ||
		    (tpc >= MODULES_VADDR && tpc < MODULES_END)) {
		} else {
			bad_kernel_pc(regs, address);
			return;
		}
	}
	if (in_atomic() || !mm)
		goto intr_or_no_mm;
	perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS, 1, 0, regs, address);
	if (!down_read_trylock(&mm->mmap_sem)) {
		if ((regs->tstate & TSTATE_PRIV) &&
		    !search_exception_tables(regs->tpc)) {
			insn = get_fault_insn(regs, insn);
			goto handle_kernel_fault;
		}
		down_read(&mm->mmap_sem);
	}
	vma = find_vma(mm, address);
	if (!vma)
		goto bad_area;
	if (((fault_code &
	      (FAULT_CODE_DTLB | FAULT_CODE_WRITE | FAULT_CODE_WINFIXUP)) == FAULT_CODE_DTLB) &&
	    (vma->vm_flags & VM_WRITE) != 0) {
		insn = get_fault_insn(regs, 0);
		if (!insn)
			goto continue_fault;
		if ((insn & 0xc0200000) == 0xc0200000 &&
		    (insn & 0x01780000) != 0x01680000) {
			fault_code |= FAULT_CODE_WRITE;
		}
	}
continue_fault:
	if (vma->vm_start <= address)
		goto good_area;
	if (!(vma->vm_flags & VM_GROWSDOWN))
		goto bad_area;
	if (!(fault_code & FAULT_CODE_WRITE)) {
		insn = get_fault_insn(regs, insn);
		if ((insn & 0xc0800000) == 0xc0800000) {
			unsigned char asi;
			if (insn & 0x2000)
				asi = (regs->tstate >> 24);
			else
				asi = (insn >> 5);
			if ((asi & 0xf2) == 0x82)
				goto bad_area;
		}
	}
	if (expand_stack(vma, address))
		goto bad_area;
good_area:
	si_code = SEGV_ACCERR;
	if ((fault_code & FAULT_CODE_ITLB) && !(vma->vm_flags & VM_EXEC)) {
		BUG_ON(address != regs->tpc);
		BUG_ON(regs->tstate & TSTATE_PRIV);
		goto bad_area;
	}
	if (fault_code & FAULT_CODE_WRITE) {
		if (!(vma->vm_flags & VM_WRITE))
			goto bad_area;
		if (tlb_type == spitfire &&
		    (vma->vm_flags & VM_EXEC) != 0 &&
		    vma->vm_file != NULL)
			set_thread_fault_code(fault_code |
					      FAULT_CODE_BLKCOMMIT);
	} else {
		if (!(vma->vm_flags & (VM_READ | VM_EXEC)))
			goto bad_area;
	}
	fault = handle_mm_fault(mm, vma, address, (fault_code & FAULT_CODE_WRITE) ? FAULT_FLAG_WRITE : 0);
	if (unlikely(fault & VM_FAULT_ERROR)) {
		if (fault & VM_FAULT_OOM)
			goto out_of_memory;
		else if (fault & VM_FAULT_SIGBUS)
			goto do_sigbus;
		BUG();
	}
	if (fault & VM_FAULT_MAJOR) {
		current->maj_flt++;
		perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS_MAJ, 1, 0,
			      regs, address);
	} else {
		current->min_flt++;
		perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS_MIN, 1, 0,
			      regs, address);
	}
	up_read(&mm->mmap_sem);
	mm_rss = get_mm_rss(mm);
#ifdef CONFIG_HUGETLB_PAGE
	mm_rss -= (mm->context.huge_pte_count * (HPAGE_SIZE / PAGE_SIZE));
#endif
	if (unlikely(mm_rss >
		     mm->context.tsb_block[MM_TSB_BASE].tsb_rss_limit))
		tsb_grow(mm, MM_TSB_BASE, mm_rss);
#ifdef CONFIG_HUGETLB_PAGE
	mm_rss = mm->context.huge_pte_count;
	if (unlikely(mm_rss >
		     mm->context.tsb_block[MM_TSB_HUGE].tsb_rss_limit))
		tsb_grow(mm, MM_TSB_HUGE, mm_rss);
#endif
	return;
bad_area:
	insn = get_fault_insn(regs, insn);
	up_read(&mm->mmap_sem);
handle_kernel_fault:
	do_kernel_fault(regs, si_code, fault_code, insn, address);
	return;
out_of_memory:
	insn = get_fault_insn(regs, insn);
	up_read(&mm->mmap_sem);
	if (!(regs->tstate & TSTATE_PRIV)) {
		pagefault_out_of_memory();
		return;
	}
	goto handle_kernel_fault;
intr_or_no_mm:
	insn = get_fault_insn(regs, 0);
	goto handle_kernel_fault;
do_sigbus:
	insn = get_fault_insn(regs, insn);
	up_read(&mm->mmap_sem);
	do_fault_siginfo(BUS_ADRERR, SIGBUS, regs, insn, fault_code);
	if (regs->tstate & TSTATE_PRIV)
		goto handle_kernel_fault;
}