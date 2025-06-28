SYSCALL_DEFINE3(rt_sigqueueinfo, pid_t, pid, int, sig,
		siginfo_t __user *, uinfo)
{
	siginfo_t info;
	if (copy_from_user(&info, uinfo, sizeof(siginfo_t)))
		return -EFAULT;
	if (info.si_code >= 0)
		return -EPERM;
	info.si_signo = sig;
	return kill_proc_info(sig, &info, pid);
}