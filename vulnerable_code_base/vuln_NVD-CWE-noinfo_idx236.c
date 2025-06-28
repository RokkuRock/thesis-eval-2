static struct nfs4_state *nfs4_do_open(struct inode *dir, struct path *path, int flags, struct iattr *sattr, struct rpc_cred *cred)
{
	struct nfs4_exception exception = { };
	struct nfs4_state *res;
	int status;
	do {
		status = _nfs4_do_open(dir, path, flags, sattr, cred, &res);
		if (status == 0)
			break;
		if (status == -NFS4ERR_BAD_SEQID) {
			printk(KERN_WARNING "NFS: v4 server %s "
					" returned a bad sequence-id error!\n",
					NFS_SERVER(dir)->nfs_client->cl_hostname);
			exception.retry = 1;
			continue;
		}
		if (status == -NFS4ERR_BAD_STATEID) {
			exception.retry = 1;
			continue;
		}
		if (status == -EAGAIN) {
			exception.retry = 1;
			continue;
		}
		res = ERR_PTR(nfs4_handle_exception(NFS_SERVER(dir),
					status, &exception));
	} while (exception.retry);
	return res;
}