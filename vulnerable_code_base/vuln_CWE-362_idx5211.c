dotraplinkage void notrace do_int3(struct pt_regs *regs, long error_code)
{
#ifdef CONFIG_DYNAMIC_FTRACE
	if (unlikely(atomic_read(&modifying_ftrace_code)) &&
	    ftrace_int3_handler(regs))
		return;
#endif
	if (poke_int3_handler(regs))
		return;
	ist_enter(regs);
	RCU_LOCKDEP_WARN(!rcu_is_watching(), "entry code didn't wake RCU");
#ifdef CONFIG_KGDB_LOW_LEVEL_TRAP
	if (kgdb_ll_trap(DIE_INT3, "int3", regs, error_code, X86_TRAP_BP,
				SIGTRAP) == NOTIFY_STOP)
		goto exit;
#endif  
#ifdef CONFIG_KPROBES
	if (kprobe_int3_handler(regs))
		goto exit;
#endif
	if (notify_die(DIE_INT3, "int3", regs, error_code, X86_TRAP_BP,
			SIGTRAP) == NOTIFY_STOP)
		goto exit;
	debug_stack_usage_inc();
	cond_local_irq_enable(regs);
	do_trap(X86_TRAP_BP, SIGTRAP, "int3", regs, error_code, NULL);
	cond_local_irq_disable(regs);
	debug_stack_usage_dec();
exit:
	ist_exit(regs);
}