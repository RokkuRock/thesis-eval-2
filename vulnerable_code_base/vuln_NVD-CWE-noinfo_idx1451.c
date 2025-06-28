long do_rt_tgsigqueueinfo(pid_t tgid, pid_t pid, int sig, siginfo_t *info)
{
	if (pid <= 0 || tgid <= 0)
		return -EINVAL;
	if (info->si_code >= 0)
		return -EPERM;
	info->si_signo = sig;
	return do_send_specific(tgid, pid, sig, info);
}