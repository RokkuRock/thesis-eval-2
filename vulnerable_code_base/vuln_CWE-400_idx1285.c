static void emulate_load_store_insn(struct pt_regs *regs,
	void __user *addr, unsigned int __user *pc)
{
	union mips_instruction insn;
	unsigned long value;
	unsigned int res;
	perf_sw_event(PERF_COUNT_SW_EMULATION_FAULTS,
		      1, 0, regs, 0);
	__get_user(insn.word, pc);
	switch (insn.i_format.opcode) {
	case ll_op:
	case lld_op:
	case sc_op:
	case scd_op:
	case ldl_op:
	case ldr_op:
	case lwl_op:
	case lwr_op:
	case sdl_op:
	case sdr_op:
	case swl_op:
	case swr_op:
	case lb_op:
	case lbu_op:
	case sb_op:
		goto sigbus;
	case lh_op:
		if (!access_ok(VERIFY_READ, addr, 2))
			goto sigbus;
		__asm__ __volatile__ (".set\tnoat\n"
#ifdef __BIG_ENDIAN
			"1:\tlb\t%0, 0(%2)\n"
			"2:\tlbu\t$1, 1(%2)\n\t"
#endif
#ifdef __LITTLE_ENDIAN
			"1:\tlb\t%0, 1(%2)\n"
			"2:\tlbu\t$1, 0(%2)\n\t"
#endif
			"sll\t%0, 0x8\n\t"
			"or\t%0, $1\n\t"
			"li\t%1, 0\n"
			"3:\t.set\tat\n\t"
			".section\t.fixup,\"ax\"\n\t"
			"4:\tli\t%1, %3\n\t"
			"j\t3b\n\t"
			".previous\n\t"
			".section\t__ex_table,\"a\"\n\t"
			STR(PTR)"\t1b, 4b\n\t"
			STR(PTR)"\t2b, 4b\n\t"
			".previous"
			: "=&r" (value), "=r" (res)
			: "r" (addr), "i" (-EFAULT));
		if (res)
			goto fault;
		compute_return_epc(regs);
		regs->regs[insn.i_format.rt] = value;
		break;
	case lw_op:
		if (!access_ok(VERIFY_READ, addr, 4))
			goto sigbus;
		__asm__ __volatile__ (
#ifdef __BIG_ENDIAN
			"1:\tlwl\t%0, (%2)\n"
			"2:\tlwr\t%0, 3(%2)\n\t"
#endif
#ifdef __LITTLE_ENDIAN
			"1:\tlwl\t%0, 3(%2)\n"
			"2:\tlwr\t%0, (%2)\n\t"
#endif
			"li\t%1, 0\n"
			"3:\t.section\t.fixup,\"ax\"\n\t"
			"4:\tli\t%1, %3\n\t"
			"j\t3b\n\t"
			".previous\n\t"
			".section\t__ex_table,\"a\"\n\t"
			STR(PTR)"\t1b, 4b\n\t"
			STR(PTR)"\t2b, 4b\n\t"
			".previous"
			: "=&r" (value), "=r" (res)
			: "r" (addr), "i" (-EFAULT));
		if (res)
			goto fault;
		compute_return_epc(regs);
		regs->regs[insn.i_format.rt] = value;
		break;
	case lhu_op:
		if (!access_ok(VERIFY_READ, addr, 2))
			goto sigbus;
		__asm__ __volatile__ (
			".set\tnoat\n"
#ifdef __BIG_ENDIAN
			"1:\tlbu\t%0, 0(%2)\n"
			"2:\tlbu\t$1, 1(%2)\n\t"
#endif
#ifdef __LITTLE_ENDIAN
			"1:\tlbu\t%0, 1(%2)\n"
			"2:\tlbu\t$1, 0(%2)\n\t"
#endif
			"sll\t%0, 0x8\n\t"
			"or\t%0, $1\n\t"
			"li\t%1, 0\n"
			"3:\t.set\tat\n\t"
			".section\t.fixup,\"ax\"\n\t"
			"4:\tli\t%1, %3\n\t"
			"j\t3b\n\t"
			".previous\n\t"
			".section\t__ex_table,\"a\"\n\t"
			STR(PTR)"\t1b, 4b\n\t"
			STR(PTR)"\t2b, 4b\n\t"
			".previous"
			: "=&r" (value), "=r" (res)
			: "r" (addr), "i" (-EFAULT));
		if (res)
			goto fault;
		compute_return_epc(regs);
		regs->regs[insn.i_format.rt] = value;
		break;
	case lwu_op:
#ifdef CONFIG_64BIT
		if (!access_ok(VERIFY_READ, addr, 4))
			goto sigbus;
		__asm__ __volatile__ (
#ifdef __BIG_ENDIAN
			"1:\tlwl\t%0, (%2)\n"
			"2:\tlwr\t%0, 3(%2)\n\t"
#endif
#ifdef __LITTLE_ENDIAN
			"1:\tlwl\t%0, 3(%2)\n"
			"2:\tlwr\t%0, (%2)\n\t"
#endif
			"dsll\t%0, %0, 32\n\t"
			"dsrl\t%0, %0, 32\n\t"
			"li\t%1, 0\n"
			"3:\t.section\t.fixup,\"ax\"\n\t"
			"4:\tli\t%1, %3\n\t"
			"j\t3b\n\t"
			".previous\n\t"
			".section\t__ex_table,\"a\"\n\t"
			STR(PTR)"\t1b, 4b\n\t"
			STR(PTR)"\t2b, 4b\n\t"
			".previous"
			: "=&r" (value), "=r" (res)
			: "r" (addr), "i" (-EFAULT));
		if (res)
			goto fault;
		compute_return_epc(regs);
		regs->regs[insn.i_format.rt] = value;
		break;
#endif  
		goto sigill;
	case ld_op:
#ifdef CONFIG_64BIT
		if (!access_ok(VERIFY_READ, addr, 8))
			goto sigbus;
		__asm__ __volatile__ (
#ifdef __BIG_ENDIAN
			"1:\tldl\t%0, (%2)\n"
			"2:\tldr\t%0, 7(%2)\n\t"
#endif
#ifdef __LITTLE_ENDIAN
			"1:\tldl\t%0, 7(%2)\n"
			"2:\tldr\t%0, (%2)\n\t"
#endif
			"li\t%1, 0\n"
			"3:\t.section\t.fixup,\"ax\"\n\t"
			"4:\tli\t%1, %3\n\t"
			"j\t3b\n\t"
			".previous\n\t"
			".section\t__ex_table,\"a\"\n\t"
			STR(PTR)"\t1b, 4b\n\t"
			STR(PTR)"\t2b, 4b\n\t"
			".previous"
			: "=&r" (value), "=r" (res)
			: "r" (addr), "i" (-EFAULT));
		if (res)
			goto fault;
		compute_return_epc(regs);
		regs->regs[insn.i_format.rt] = value;
		break;
#endif  
		goto sigill;
	case sh_op:
		if (!access_ok(VERIFY_WRITE, addr, 2))
			goto sigbus;
		value = regs->regs[insn.i_format.rt];
		__asm__ __volatile__ (
#ifdef __BIG_ENDIAN
			".set\tnoat\n"
			"1:\tsb\t%1, 1(%2)\n\t"
			"srl\t$1, %1, 0x8\n"
			"2:\tsb\t$1, 0(%2)\n\t"
			".set\tat\n\t"
#endif
#ifdef __LITTLE_ENDIAN
			".set\tnoat\n"
			"1:\tsb\t%1, 0(%2)\n\t"
			"srl\t$1,%1, 0x8\n"
			"2:\tsb\t$1, 1(%2)\n\t"
			".set\tat\n\t"
#endif
			"li\t%0, 0\n"
			"3:\n\t"
			".section\t.fixup,\"ax\"\n\t"
			"4:\tli\t%0, %3\n\t"
			"j\t3b\n\t"
			".previous\n\t"
			".section\t__ex_table,\"a\"\n\t"
			STR(PTR)"\t1b, 4b\n\t"
			STR(PTR)"\t2b, 4b\n\t"
			".previous"
			: "=r" (res)
			: "r" (value), "r" (addr), "i" (-EFAULT));
		if (res)
			goto fault;
		compute_return_epc(regs);
		break;
	case sw_op:
		if (!access_ok(VERIFY_WRITE, addr, 4))
			goto sigbus;
		value = regs->regs[insn.i_format.rt];
		__asm__ __volatile__ (
#ifdef __BIG_ENDIAN
			"1:\tswl\t%1,(%2)\n"
			"2:\tswr\t%1, 3(%2)\n\t"
#endif
#ifdef __LITTLE_ENDIAN
			"1:\tswl\t%1, 3(%2)\n"
			"2:\tswr\t%1, (%2)\n\t"
#endif
			"li\t%0, 0\n"
			"3:\n\t"
			".section\t.fixup,\"ax\"\n\t"
			"4:\tli\t%0, %3\n\t"
			"j\t3b\n\t"
			".previous\n\t"
			".section\t__ex_table,\"a\"\n\t"
			STR(PTR)"\t1b, 4b\n\t"
			STR(PTR)"\t2b, 4b\n\t"
			".previous"
		: "=r" (res)
		: "r" (value), "r" (addr), "i" (-EFAULT));
		if (res)
			goto fault;
		compute_return_epc(regs);
		break;
	case sd_op:
#ifdef CONFIG_64BIT
		if (!access_ok(VERIFY_WRITE, addr, 8))
			goto sigbus;
		value = regs->regs[insn.i_format.rt];
		__asm__ __volatile__ (
#ifdef __BIG_ENDIAN
			"1:\tsdl\t%1,(%2)\n"
			"2:\tsdr\t%1, 7(%2)\n\t"
#endif
#ifdef __LITTLE_ENDIAN
			"1:\tsdl\t%1, 7(%2)\n"
			"2:\tsdr\t%1, (%2)\n\t"
#endif
			"li\t%0, 0\n"
			"3:\n\t"
			".section\t.fixup,\"ax\"\n\t"
			"4:\tli\t%0, %3\n\t"
			"j\t3b\n\t"
			".previous\n\t"
			".section\t__ex_table,\"a\"\n\t"
			STR(PTR)"\t1b, 4b\n\t"
			STR(PTR)"\t2b, 4b\n\t"
			".previous"
		: "=r" (res)
		: "r" (value), "r" (addr), "i" (-EFAULT));
		if (res)
			goto fault;
		compute_return_epc(regs);
		break;
#endif  
		goto sigill;
	case lwc1_op:
	case ldc1_op:
	case swc1_op:
	case sdc1_op:
		goto sigbus;
	case lwc2_op:
		cu2_notifier_call_chain(CU2_LWC2_OP, regs);
		break;
	case ldc2_op:
		cu2_notifier_call_chain(CU2_LDC2_OP, regs);
		break;
	case swc2_op:
		cu2_notifier_call_chain(CU2_SWC2_OP, regs);
		break;
	case sdc2_op:
		cu2_notifier_call_chain(CU2_SDC2_OP, regs);
		break;
	default:
		goto sigill;
	}
#ifdef CONFIG_DEBUG_FS
	unaligned_instructions++;
#endif
	return;
fault:
	if (fixup_exception(regs))
		return;
	die_if_kernel("Unhandled kernel unaligned access", regs);
	force_sig(SIGSEGV, current);
	return;
sigbus:
	die_if_kernel("Unhandled kernel unaligned access", regs);
	force_sig(SIGBUS, current);
	return;
sigill:
	die_if_kernel("Unhandled kernel unaligned access or invalid instruction", regs);
	force_sig(SIGILL, current);
}