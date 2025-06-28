static int ptrace_check_attach(struct task_struct *child, bool ignore_state)
{
	int ret = -ESRCH;
	read_lock(&tasklist_lock);
	if ((child->ptrace & PT_PTRACED) && child->parent == current) {
		spin_lock_irq(&child->sighand->siglock);
		WARN_ON_ONCE(task_is_stopped(child));
		if (ignore_state || (task_is_traced(child) &&
				     !(child->jobctl & JOBCTL_LISTENING)))
			ret = 0;
		spin_unlock_irq(&child->sighand->siglock);
	}
	read_unlock(&tasklist_lock);
	if (!ret && !ignore_state)
		ret = wait_task_inactive(child, TASK_TRACED) ? 0 : -ESRCH;
	return ret;
}