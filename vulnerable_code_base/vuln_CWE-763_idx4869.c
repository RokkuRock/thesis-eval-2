raw_copy_to_user(void __user *dst, const void *src, unsigned long size)
{
	int ret = 0;
	if (!__builtin_constant_p(size))
		return copy_user_generic((__force void *)dst, src, size);
	switch (size) {
	case 1:
		__uaccess_begin();
		__put_user_asm(*(u8 *)src, (u8 __user *)dst,
			      ret, "b", "b", "iq", 1);
		__uaccess_end();
		return ret;
	case 2:
		__uaccess_begin();
		__put_user_asm(*(u16 *)src, (u16 __user *)dst,
			      ret, "w", "w", "ir", 2);
		__uaccess_end();
		return ret;
	case 4:
		__uaccess_begin();
		__put_user_asm(*(u32 *)src, (u32 __user *)dst,
			      ret, "l", "k", "ir", 4);
		__uaccess_end();
		return ret;
	case 8:
		__uaccess_begin();
		__put_user_asm(*(u64 *)src, (u64 __user *)dst,
			      ret, "q", "", "er", 8);
		__uaccess_end();
		return ret;
	case 10:
		__uaccess_begin();
		__put_user_asm(*(u64 *)src, (u64 __user *)dst,
			       ret, "q", "", "er", 10);
		if (likely(!ret)) {
			asm("":::"memory");
			__put_user_asm(4[(u16 *)src], 4 + (u16 __user *)dst,
				       ret, "w", "w", "ir", 2);
		}
		__uaccess_end();
		return ret;
	case 16:
		__uaccess_begin();
		__put_user_asm(*(u64 *)src, (u64 __user *)dst,
			       ret, "q", "", "er", 16);
		if (likely(!ret)) {
			asm("":::"memory");
			__put_user_asm(1[(u64 *)src], 1 + (u64 __user *)dst,
				       ret, "q", "", "er", 8);
		}
		__uaccess_end();
		return ret;
	default:
		return copy_user_generic((__force void *)dst, src, size);
	}
}