int pipe_resize_ring(struct pipe_inode_info *pipe, unsigned int nr_slots)
{
	struct pipe_buffer *bufs;
	unsigned int head, tail, mask, n;
	mask = pipe->ring_size - 1;
	head = pipe->head;
	tail = pipe->tail;
	n = pipe_occupancy(pipe->head, pipe->tail);
	if (nr_slots < n)
		return -EBUSY;
	bufs = kcalloc(nr_slots, sizeof(*bufs),
		       GFP_KERNEL_ACCOUNT | __GFP_NOWARN);
	if (unlikely(!bufs))
		return -ENOMEM;
	if (n > 0) {
		unsigned int h = head & mask;
		unsigned int t = tail & mask;
		if (h > t) {
			memcpy(bufs, pipe->bufs + t,
			       n * sizeof(struct pipe_buffer));
		} else {
			unsigned int tsize = pipe->ring_size - t;
			if (h > 0)
				memcpy(bufs + tsize, pipe->bufs,
				       h * sizeof(struct pipe_buffer));
			memcpy(bufs, pipe->bufs + t,
			       tsize * sizeof(struct pipe_buffer));
		}
	}
	head = n;
	tail = 0;
	kfree(pipe->bufs);
	pipe->bufs = bufs;
	pipe->ring_size = nr_slots;
	if (pipe->max_usage > nr_slots)
		pipe->max_usage = nr_slots;
	pipe->tail = tail;
	pipe->head = head;
	wake_up_interruptible(&pipe->wr_wait);
	return 0;
}