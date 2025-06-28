static inline int xsave_state(struct xsave_struct *fx, u64 mask)
{
	u32 lmask = mask;
	u32 hmask = mask >> 32;
	int err = 0;
	alternative_input_2(
		"1:"XSAVE,
		"1:"XSAVEOPT,
		X86_FEATURE_XSAVEOPT,
		"1:"XSAVES,
		X86_FEATURE_XSAVES,
		[fx] "D" (fx), "a" (lmask), "d" (hmask) :
		"memory");
	asm volatile("2:\n\t"
		     xstate_fault
		     : "0" (0)
		     : "memory");
	return err;
}