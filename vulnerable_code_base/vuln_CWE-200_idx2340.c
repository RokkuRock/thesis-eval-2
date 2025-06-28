static int load_misc_binary(struct linux_binprm *bprm)
{
	Node *fmt;
	struct file * interp_file = NULL;
	char iname[BINPRM_BUF_SIZE];
	const char *iname_addr = iname;
	int retval;
	int fd_binary = -1;
	retval = -ENOEXEC;
	if (!enabled)
		goto _ret;
	read_lock(&entries_lock);
	fmt = check_file(bprm);
	if (fmt)
		strlcpy(iname, fmt->interpreter, BINPRM_BUF_SIZE);
	read_unlock(&entries_lock);
	if (!fmt)
		goto _ret;
	if (!(fmt->flags & MISC_FMT_PRESERVE_ARGV0)) {
		retval = remove_arg_zero(bprm);
		if (retval)
			goto _ret;
	}
	if (fmt->flags & MISC_FMT_OPEN_BINARY) {
 		fd_binary = get_unused_fd();
 		if (fd_binary < 0) {
 			retval = fd_binary;
 			goto _ret;
 		}
 		fd_install(fd_binary, bprm->file);
		would_dump(bprm, bprm->file);
		allow_write_access(bprm->file);
		bprm->file = NULL;
		bprm->interp_flags |= BINPRM_FLAGS_EXECFD;
		bprm->interp_data = fd_binary;
 	} else {
 		allow_write_access(bprm->file);
 		fput(bprm->file);
 		bprm->file = NULL;
 	}
	retval = copy_strings_kernel (1, &bprm->interp, bprm);
	if (retval < 0)
		goto _error;
	bprm->argc++;
	retval = copy_strings_kernel (1, &iname_addr, bprm);
	if (retval < 0)
		goto _error;
	bprm->argc ++;
	bprm->interp = iname;	 
	interp_file = open_exec (iname);
	retval = PTR_ERR (interp_file);
	if (IS_ERR (interp_file))
		goto _error;
	bprm->file = interp_file;
	if (fmt->flags & MISC_FMT_CREDENTIALS) {
		memset(bprm->buf, 0, BINPRM_BUF_SIZE);
		retval = kernel_read(bprm->file, 0, bprm->buf, BINPRM_BUF_SIZE);
	} else
		retval = prepare_binprm (bprm);
	if (retval < 0)
		goto _error;
	retval = search_binary_handler(bprm);
	if (retval < 0)
		goto _error;
_ret:
	return retval;
_error:
	if (fd_binary > 0)
		sys_close(fd_binary);
	bprm->interp_flags = 0;
	bprm->interp_data = 0;
	goto _ret;
}