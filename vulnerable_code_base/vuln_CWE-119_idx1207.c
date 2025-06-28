ssh_packet_set_compress_hooks(struct ssh *ssh, void *ctx,
    void *(*allocfunc)(void *, u_int, u_int),
    void (*freefunc)(void *, void *))
{
	ssh->state->compression_out_stream.zalloc = (alloc_func)allocfunc;
	ssh->state->compression_out_stream.zfree = (free_func)freefunc;
	ssh->state->compression_out_stream.opaque = ctx;
	ssh->state->compression_in_stream.zalloc = (alloc_func)allocfunc;
	ssh->state->compression_in_stream.zfree = (free_func)freefunc;
	ssh->state->compression_in_stream.opaque = ctx;
}