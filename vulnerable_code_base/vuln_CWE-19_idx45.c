void altivec_unavailable_exception(struct pt_regs *regs)
{
#if !defined(CONFIG_ALTIVEC)
	if (user_mode(regs)) {
		_exception(SIGILL, regs, ILL_ILLOPC, regs->nip);
		return;
	}
#endif
	printk(KERN_EMERG "Unrecoverable VMX/Altivec Unavailable Exception "
			"%lx at %lx\n", regs->trap, regs->nip);
	die("Unrecoverable VMX/Altivec Unavailable Exception", regs, SIGABRT);
}