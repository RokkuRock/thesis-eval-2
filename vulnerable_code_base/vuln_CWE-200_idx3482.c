__switch_to(struct task_struct *prev_p, struct task_struct *next_p)
{
	struct thread_struct *prev = &prev_p->thread;
	struct thread_struct *next = &next_p->thread;
	int cpu = smp_processor_id();
	struct tss_struct *tss = &per_cpu(init_tss, cpu);
	unsigned fsindex, gsindex;
	fpu_switch_t fpu;
	fpu = switch_fpu_prepare(prev_p, next_p, cpu);
	load_sp0(tss, next);
	savesegment(es, prev->es);
	if (unlikely(next->es | prev->es))
		loadsegment(es, next->es);
	savesegment(ds, prev->ds);
	if (unlikely(next->ds | prev->ds))
		loadsegment(ds, next->ds);
	savesegment(fs, fsindex);
	savesegment(gs, gsindex);
	load_TLS(next, cpu);
	arch_end_context_switch(next_p);
	if (unlikely(fsindex | next->fsindex | prev->fs)) {
		loadsegment(fs, next->fsindex);
		if (fsindex)
			prev->fs = 0;
	}
	if (next->fs)
		wrmsrl(MSR_FS_BASE, next->fs);
	prev->fsindex = fsindex;
	if (unlikely(gsindex | next->gsindex | prev->gs)) {
		load_gs_index(next->gsindex);
		if (gsindex)
			prev->gs = 0;
	}
	if (next->gs)
		wrmsrl(MSR_KERNEL_GS_BASE, next->gs);
	prev->gsindex = gsindex;
	switch_fpu_finish(next_p, fpu);
	prev->usersp = this_cpu_read(old_rsp);
	this_cpu_write(old_rsp, next->usersp);
	this_cpu_write(current_task, next_p);
	task_thread_info(prev_p)->saved_preempt_count = this_cpu_read(__preempt_count);
	this_cpu_write(__preempt_count, task_thread_info(next_p)->saved_preempt_count);
	this_cpu_write(kernel_stack,
		  (unsigned long)task_stack_page(next_p) +
		  THREAD_SIZE - KERNEL_STACK_OFFSET);
	if (unlikely(task_thread_info(next_p)->flags & _TIF_WORK_CTXSW_NEXT ||
		     task_thread_info(prev_p)->flags & _TIF_WORK_CTXSW_PREV))
		__switch_to_xtra(prev_p, next_p, tss);
	return prev_p;
}