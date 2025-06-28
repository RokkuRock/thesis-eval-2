static int vt_k_ioctl(struct tty_struct *tty, unsigned int cmd,
		unsigned long arg, bool perm)
{
	struct vc_data *vc = tty->driver_data;
	void __user *up = (void __user *)arg;
	unsigned int console = vc->vc_num;
	int ret;
	switch (cmd) {
	case KIOCSOUND:
		if (!perm)
			return -EPERM;
		if (arg)
			arg = PIT_TICK_RATE / arg;
		kd_mksound(arg, 0);
		break;
	case KDMKTONE:
		if (!perm)
			return -EPERM;
	{
		unsigned int ticks, count;
		ticks = msecs_to_jiffies((arg >> 16) & 0xffff);
		count = ticks ? (arg & 0xffff) : 0;
		if (count)
			count = PIT_TICK_RATE / count;
		kd_mksound(count, ticks);
		break;
	}
	case KDGKBTYPE:
		return put_user(KB_101, (char __user *)arg);
#ifdef CONFIG_X86
	case KDADDIO:
	case KDDELIO:
		if (arg < GPFIRST || arg > GPLAST)
			return -EINVAL;
		return ksys_ioperm(arg, 1, (cmd == KDADDIO)) ? -ENXIO : 0;
	case KDENABIO:
	case KDDISABIO:
		return ksys_ioperm(GPFIRST, GPNUM,
				  (cmd == KDENABIO)) ? -ENXIO : 0;
#endif
	case KDKBDREP:
	{
		struct kbd_repeat kbrep;
		if (!capable(CAP_SYS_TTY_CONFIG))
			return -EPERM;
		if (copy_from_user(&kbrep, up, sizeof(struct kbd_repeat)))
			return -EFAULT;
		ret = kbd_rate(&kbrep);
		if (ret)
			return ret;
		if (copy_to_user(up, &kbrep, sizeof(struct kbd_repeat)))
			return -EFAULT;
		break;
	}
	case KDSETMODE:
		if (!perm)
			return -EPERM;
		return vt_kdsetmode(vc, arg);
	case KDGETMODE:
		return put_user(vc->vc_mode, (int __user *)arg);
	case KDMAPDISP:
	case KDUNMAPDISP:
		return -EINVAL;
	case KDSKBMODE:
		if (!perm)
			return -EPERM;
		ret = vt_do_kdskbmode(console, arg);
		if (ret)
			return ret;
		tty_ldisc_flush(tty);
		break;
	case KDGKBMODE:
		return put_user(vt_do_kdgkbmode(console), (int __user *)arg);
	case KDSKBMETA:
		return vt_do_kdskbmeta(console, arg);
	case KDGKBMETA:
		return put_user(vt_do_kdgkbmeta(console), (int __user *)arg);
	case KDGETKEYCODE:
	case KDSETKEYCODE:
		if(!capable(CAP_SYS_TTY_CONFIG))
			perm = 0;
		return vt_do_kbkeycode_ioctl(cmd, up, perm);
	case KDGKBENT:
	case KDSKBENT:
		return vt_do_kdsk_ioctl(cmd, up, perm, console);
	case KDGKBSENT:
	case KDSKBSENT:
		return vt_do_kdgkb_ioctl(cmd, up, perm);
	case KDGKBDIACR:
	case KDGKBDIACRUC:
	case KDSKBDIACR:
	case KDSKBDIACRUC:
		return vt_do_diacrit(cmd, up, perm);
	case KDGKBLED:
	case KDSKBLED:
	case KDGETLED:
	case KDSETLED:
		return vt_do_kdskled(console, cmd, arg, perm);
	case KDSIGACCEPT:
		if (!perm || !capable(CAP_KILL))
			return -EPERM;
		if (!valid_signal(arg) || arg < 1 || arg == SIGKILL)
			return -EINVAL;
		spin_lock_irq(&vt_spawn_con.lock);
		put_pid(vt_spawn_con.pid);
		vt_spawn_con.pid = get_pid(task_pid(current));
		vt_spawn_con.sig = arg;
		spin_unlock_irq(&vt_spawn_con.lock);
		break;
	case KDFONTOP: {
		struct console_font_op op;
		if (copy_from_user(&op, up, sizeof(op)))
			return -EFAULT;
		if (!perm && op.op != KD_FONT_OP_GET)
			return -EPERM;
		ret = con_font_op(vc, &op);
		if (ret)
			return ret;
		if (copy_to_user(up, &op, sizeof(op)))
			return -EFAULT;
		break;
	}
	default:
		return -ENOIOCTLCMD;
	}
	return 0;
}