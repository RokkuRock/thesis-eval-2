static int link_pipe(struct pipe_inode_info *ipipe,
		     struct pipe_inode_info *opipe,
		     size_t len, unsigned int flags)
{
	struct pipe_buffer *ibuf, *obuf;
	int ret = 0, i = 0, nbuf;
	pipe_double_lock(ipipe, opipe);
	do {
		if (!opipe->readers) {
			send_sig(SIGPIPE, current, 0);
			if (!ret)
				ret = -EPIPE;
			break;
		}
		if (i >= ipipe->nrbufs || opipe->nrbufs >= opipe->buffers)
			break;
		ibuf = ipipe->bufs + ((ipipe->curbuf + i) & (ipipe->buffers-1));
		nbuf = (opipe->curbuf + opipe->nrbufs) & (opipe->buffers - 1);
		pipe_buf_get(ipipe, ibuf);
		obuf = opipe->bufs + nbuf;
		*obuf = *ibuf;
		obuf->flags &= ~PIPE_BUF_FLAG_GIFT;
		if (obuf->len > len)
			obuf->len = len;
		opipe->nrbufs++;
		ret += obuf->len;
		len -= obuf->len;
		i++;
	} while (len);
	if (!ret && ipipe->waiting_writers && (flags & SPLICE_F_NONBLOCK))
		ret = -EAGAIN;
	pipe_unlock(ipipe);
	pipe_unlock(opipe);
	if (ret > 0)
		wakeup_pipe_readers(opipe);
	return ret;
}