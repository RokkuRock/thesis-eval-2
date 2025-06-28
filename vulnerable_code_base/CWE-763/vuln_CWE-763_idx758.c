raw_copy_from_user(void *dst, const void __user *src, unsigned long size)
{
	int ret = 0;
	if (!__builtin_constant_p(size))
		return copy_user_generic(dst, (__force void *)src, size);
	switch (size) {
	case 1:
		__uaccess_begin_nospec();
		__get_user_asm_nozero(*(u8 *)dst, (u8 __user *)src,
			      ret, "b", "b", "=q", 1);
		__uaccess_end();
		return ret;
	case 2:
		__uaccess_begin_nospec();
		__get_user_asm_nozero(*(u16 *)dst, (u16 __user *)src,
			      ret, "w", "w", "=r", 2);
		__uaccess_end();
		return ret;
	case 4:
		__uaccess_begin_nospec();
		__get_user_asm_nozero(*(u32 *)dst, (u32 __user *)src,
			      ret, "l", "k", "=r", 4);
		__uaccess_end();
		return ret;
	case 8:
		__uaccess_begin_nospec();
		__get_user_asm_nozero(*(u64 *)dst, (u64 __user *)src,
			      ret, "q", "", "=r", 8);
		__uaccess_end();
		return ret;
	case 10:
		__uaccess_begin_nospec();
		__get_user_asm_nozero(*(u64 *)dst, (u64 __user *)src,
			       ret, "q", "", "=r", 10);
		if (likely(!ret))
			__get_user_asm_nozero(*(u16 *)(8 + (char *)dst),
				       (u16 __user *)(8 + (char __user *)src),
				       ret, "w", "w", "=r", 2);
		__uaccess_end();
		return ret;
	case 16:
		__uaccess_begin_nospec();
		__get_user_asm_nozero(*(u64 *)dst, (u64 __user *)src,
			       ret, "q", "", "=r", 16);
		if (likely(!ret))
			__get_user_asm_nozero(*(u64 *)(8 + (char *)dst),
				       (u64 __user *)(8 + (char __user *)src),
				       ret, "q", "", "=r", 8);
		__uaccess_end();
		return ret;
	default:
		return copy_user_generic(dst, (__force void *)src, size);
	}
}