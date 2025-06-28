SYSCALL_DEFINE4(epoll_ctl, int, epfd, int, op, int, fd,
		struct epoll_event __user *, event)
{
	int error;
	int did_lock_epmutex = 0;
	struct file *file, *tfile;
	struct eventpoll *ep;
	struct epitem *epi;
	struct epoll_event epds;
	error = -EFAULT;
	if (ep_op_has_event(op) &&
	    copy_from_user(&epds, event, sizeof(struct epoll_event)))
		goto error_return;
	error = -EBADF;
	file = fget(epfd);
	if (!file)
		goto error_return;
	tfile = fget(fd);
	if (!tfile)
		goto error_fput;
	error = -EPERM;
	if (!tfile->f_op || !tfile->f_op->poll)
		goto error_tgt_fput;
	error = -EINVAL;
	if (file == tfile || !is_file_epoll(file))
		goto error_tgt_fput;
	ep = file->private_data;
	if (op == EPOLL_CTL_ADD || op == EPOLL_CTL_DEL) {
		mutex_lock(&epmutex);
		did_lock_epmutex = 1;
	}
	if (op == EPOLL_CTL_ADD) {
		if (is_file_epoll(tfile)) {
			error = -ELOOP;
			if (ep_loop_check(ep, tfile) != 0)
				goto error_tgt_fput;
		} else
			list_add(&tfile->f_tfile_llink, &tfile_check_list);
	}
	mutex_lock_nested(&ep->mtx, 0);
	epi = ep_find(ep, tfile, fd);
	error = -EINVAL;
	switch (op) {
	case EPOLL_CTL_ADD:
		if (!epi) {
			epds.events |= POLLERR | POLLHUP;
			error = ep_insert(ep, &epds, tfile, fd);
		} else
			error = -EEXIST;
		clear_tfile_check_list();
		break;
	case EPOLL_CTL_DEL:
		if (epi)
			error = ep_remove(ep, epi);
		else
			error = -ENOENT;
		break;
	case EPOLL_CTL_MOD:
		if (epi) {
			epds.events |= POLLERR | POLLHUP;
			error = ep_modify(ep, epi, &epds);
		} else
			error = -ENOENT;
		break;
	}
	mutex_unlock(&ep->mtx);
error_tgt_fput:
	if (did_lock_epmutex)
		mutex_unlock(&epmutex);
	fput(tfile);
error_fput:
	fput(file);
error_return:
	return error;
}