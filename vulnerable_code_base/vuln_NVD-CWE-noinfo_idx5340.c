static inline void encode_openhdr(struct xdr_stream *xdr, const struct nfs_openargs *arg)
{
	__be32 *p;
	RESERVE_SPACE(8);
	WRITE32(OP_OPEN);
	WRITE32(arg->seqid->sequence->counter);
	encode_share_access(xdr, arg->open_flags);
	RESERVE_SPACE(28);
	WRITE64(arg->clientid);
	WRITE32(16);
	WRITEMEM("open id:", 8);
	WRITE64(arg->id);
}