static void asmlinkage smm_do_relocation(void *arg)
{
	const struct smm_module_params *p;
	const struct smm_runtime *runtime;
	int cpu;
	uintptr_t curr_smbase;
	uintptr_t perm_smbase;
	p = arg;
	runtime = p->runtime;
	cpu = p->cpu;
	curr_smbase = runtime->smbase;
	if (cpu >= CONFIG_MAX_CPUS) {
		printk(BIOS_CRIT,
		       "Invalid CPU number assigned in SMM stub: %d\n", cpu);
		return;
	}
	perm_smbase = mp_state.perm_smbase;
	perm_smbase -= cpu * runtime->save_state_size;
	printk(BIOS_DEBUG, "New SMBASE 0x%08lx\n", perm_smbase);
	mp_state.ops.relocation_handler(cpu, curr_smbase, perm_smbase);
	if (CONFIG(STM)) {
		uintptr_t mseg;
		mseg = mp_state.perm_smbase +
			(mp_state.perm_smsize - CONFIG_MSEG_SIZE);
		stm_setup(mseg, p->cpu,
				perm_smbase,
				mp_state.perm_smbase,
				runtime->start32_offset);
	}
}