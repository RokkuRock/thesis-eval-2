long video_ioctl2(struct file *file,
	       unsigned int cmd, unsigned long arg)
{
	char	sbuf[128];
	void    *mbuf = NULL;
	void	*parg = (void *)arg;
	long	err  = -EINVAL;
	bool	has_array_args;
	size_t  array_size = 0;
	void __user *user_ptr = NULL;
	void	**kernel_ptr = NULL;
	if (_IOC_DIR(cmd) != _IOC_NONE) {
		if (_IOC_SIZE(cmd) <= sizeof(sbuf)) {
			parg = sbuf;
		} else {
			mbuf = kmalloc(_IOC_SIZE(cmd), GFP_KERNEL);
			if (NULL == mbuf)
				return -ENOMEM;
			parg = mbuf;
		}
		err = -EFAULT;
		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			unsigned long n = cmd_input_size(cmd);
			if (copy_from_user(parg, (void __user *)arg, n))
				goto out;
			if (n < _IOC_SIZE(cmd))
				memset((u8 *)parg + n, 0, _IOC_SIZE(cmd) - n);
		} else {
			memset(parg, 0, _IOC_SIZE(cmd));
		}
	}
	err = check_array_args(cmd, parg, &array_size, &user_ptr, &kernel_ptr);
	if (err < 0)
		goto out;
	has_array_args = err;
	if (has_array_args) {
		mbuf = kmalloc(array_size, GFP_KERNEL);
		err = -ENOMEM;
		if (NULL == mbuf)
			goto out_array_args;
		err = -EFAULT;
		if (copy_from_user(mbuf, user_ptr, array_size))
			goto out_array_args;
		*kernel_ptr = mbuf;
	}
	err = __video_do_ioctl(file, cmd, parg);
	if (err == -ENOIOCTLCMD)
		err = -EINVAL;
	if (has_array_args) {
		*kernel_ptr = user_ptr;
		if (copy_to_user(user_ptr, mbuf, array_size))
			err = -EFAULT;
		goto out_array_args;
	}
	if (err < 0)
		goto out;
out_array_args:
	switch (_IOC_DIR(cmd)) {
	case _IOC_READ:
	case (_IOC_WRITE | _IOC_READ):
		if (copy_to_user((void __user *)arg, parg, _IOC_SIZE(cmd)))
			err = -EFAULT;
		break;
	}
out:
	kfree(mbuf);
	return err;
}