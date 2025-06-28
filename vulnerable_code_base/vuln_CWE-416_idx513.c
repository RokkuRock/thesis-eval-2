void vmacache_flush_all(struct mm_struct *mm)
{
	struct task_struct *g, *p;
	count_vm_vmacache_event(VMACACHE_FULL_FLUSHES);
	if (atomic_read(&mm->mm_users) == 1)
		return;
	rcu_read_lock();
	for_each_process_thread(g, p) {
		if (mm == p->mm)
			vmacache_flush(p);
	}
	rcu_read_unlock();
}