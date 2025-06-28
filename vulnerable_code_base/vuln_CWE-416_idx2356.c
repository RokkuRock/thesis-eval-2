static int splice_pipe_to_pipe(struct pipe_inode_info *ipipe,
			       struct pipe_inode_info *opipe,
			       size_t len, unsigned int flags)
{
	struct pipe_buffer *ibuf, *obuf;
	int ret = 0, nbuf;
	bool input_wakeup = false;
retry:
	ret = ipipe_prep(ipipe, flags);
	if (ret)
		return ret;
	ret = opipe_prep(opipe, flags);
	if (ret)
		return ret;
	pipe_double_lock(ipipe, opipe);
	do {
		if (!opipe->readers) {
			send_sig(SIGPIPE, current, 0);
			if (!ret)
				ret = -EPIPE;
			break;
		}
		if (!ipipe->nrbufs && !ipipe->writers)
			break;
		if (!ipipe->nrbufs || opipe->nrbufs >= opipe->buffers) {
			if (ret)
				break;
			if (flags & SPLICE_F_NONBLOCK) {
				ret = -EAGAIN;
				break;
			}
			pipe_unlock(ipipe);
			pipe_unlock(opipe);
			goto retry;
		}
		ibuf = ipipe->bufs + ipipe->curbuf;
		nbuf = (opipe->curbuf + opipe->nrbufs) & (opipe->buffers - 1);
		obuf = opipe->bufs + nbuf;
		if (len >= ibuf->len) {
			*obuf = *ibuf;
			ibuf->ops = NULL;
			opipe->nrbufs++;
			ipipe->curbuf = (ipipe->curbuf + 1) & (ipipe->buffers - 1);
			ipipe->nrbufs--;
			input_wakeup = true;
		} else {
			pipe_buf_get(ipipe, ibuf);
			*obuf = *ibuf;
			obuf->flags &= ~PIPE_BUF_FLAG_GIFT;
			obuf->len = len;
			opipe->nrbufs++;
			ibuf->offset += obuf->len;
			ibuf->len -= obuf->len;
		}
		ret += obuf->len;
		len -= obuf->len;
	} while (len);
	pipe_unlock(ipipe);
	pipe_unlock(opipe);
	if (ret > 0)
		wakeup_pipe_readers(opipe);
	if (input_wakeup)
		wakeup_pipe_writers(ipipe);
	return ret;
}