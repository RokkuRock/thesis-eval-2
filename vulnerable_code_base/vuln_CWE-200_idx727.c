unsigned paravirt_patch_jmp(void *insnbuf, const void *target,
			    unsigned long addr, unsigned len)
{
	struct branch *b = insnbuf;
	unsigned long delta = (unsigned long)target - (addr+5);
	if (len < 5)
		return len;	 
	b->opcode = 0xe9;	 
	b->delta = delta;
	return 5;
}