static void binder_deferred_fd_close(int fd)
{
	struct binder_task_work_cb *twcb;
	twcb = kzalloc(sizeof(*twcb), GFP_KERNEL);
	if (!twcb)
		return;
	init_task_work(&twcb->twork, binder_do_fd_close);
	__close_fd_get_file(fd, &twcb->file);
	if (twcb->file)
		task_work_add(current, &twcb->twork, TWA_RESUME);
	else
		kfree(twcb);
}