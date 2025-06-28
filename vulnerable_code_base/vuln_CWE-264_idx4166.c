static inline int restore_fpu_checking(struct task_struct *tsk)
{
	alternative_input(
		ASM_NOP8 ASM_NOP2,
		"emms\n\t"		 
		"fildl %P[addr]",	 
		X86_FEATURE_FXSAVE_LEAK,
		[addr] "m" (tsk->thread.fpu.has_fpu));
	return fpu_restore_checking(&tsk->thread.fpu);
}