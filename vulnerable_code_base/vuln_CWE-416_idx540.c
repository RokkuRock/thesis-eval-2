static bool __oom_reap_task_mm(struct task_struct *tsk, struct mm_struct *mm)
{
	struct mmu_gather tlb;
	struct vm_area_struct *vma;
	bool ret = true;
	mutex_lock(&oom_lock);
	if (!down_read_trylock(&mm->mmap_sem)) {
		ret = false;
		trace_skip_task_reaping(tsk->pid);
		goto unlock_oom;
	}
	if (mm_has_notifiers(mm)) {
		up_read(&mm->mmap_sem);
		schedule_timeout_idle(HZ);
		goto unlock_oom;
	}
	if (test_bit(MMF_OOM_SKIP, &mm->flags)) {
		up_read(&mm->mmap_sem);
		trace_skip_task_reaping(tsk->pid);
		goto unlock_oom;
	}
	trace_start_task_reaping(tsk->pid);
	set_bit(MMF_UNSTABLE, &mm->flags);
	tlb_gather_mmu(&tlb, mm, 0, -1);
	for (vma = mm->mmap ; vma; vma = vma->vm_next) {
		if (!can_madv_dontneed_vma(vma))
			continue;
		if (vma_is_anonymous(vma) || !(vma->vm_flags & VM_SHARED))
			unmap_page_range(&tlb, vma, vma->vm_start, vma->vm_end,
					 NULL);
	}
	tlb_finish_mmu(&tlb, 0, -1);
	pr_info("oom_reaper: reaped process %d (%s), now anon-rss:%lukB, file-rss:%lukB, shmem-rss:%lukB\n",
			task_pid_nr(tsk), tsk->comm,
			K(get_mm_counter(mm, MM_ANONPAGES)),
			K(get_mm_counter(mm, MM_FILEPAGES)),
			K(get_mm_counter(mm, MM_SHMEMPAGES)));
	up_read(&mm->mmap_sem);
	trace_finish_task_reaping(tsk->pid);
unlock_oom:
	mutex_unlock(&oom_lock);
	return ret;
}