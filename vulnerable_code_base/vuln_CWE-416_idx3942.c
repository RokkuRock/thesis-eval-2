int __close_fd_get_file(unsigned int fd, struct file **res)
{
	struct files_struct *files = current->files;
	struct file *file;
	struct fdtable *fdt;
	spin_lock(&files->file_lock);
	fdt = files_fdtable(files);
	if (fd >= fdt->max_fds)
		goto out_unlock;
	file = fdt->fd[fd];
	if (!file)
		goto out_unlock;
	rcu_assign_pointer(fdt->fd[fd], NULL);
	__put_unused_fd(files, fd);
	spin_unlock(&files->file_lock);
	get_file(file);
	*res = file;
	return filp_close(file, files);
out_unlock:
	spin_unlock(&files->file_lock);
	*res = NULL;
	return -ENOENT;
}