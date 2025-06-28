static int __rds_rdma_map(struct rds_sock *rs, struct rds_get_mr_args *args,
				u64 *cookie_ret, struct rds_mr **mr_ret)
{
	struct rds_mr *mr = NULL, *found;
	unsigned int nr_pages;
	struct page **pages = NULL;
	struct scatterlist *sg;
	void *trans_private;
	unsigned long flags;
	rds_rdma_cookie_t cookie;
	unsigned int nents;
	long i;
	int ret;
	if (rs->rs_bound_addr == 0) {
		ret = -ENOTCONN;  
		goto out;
	}
	if (!rs->rs_transport->get_mr) {
		ret = -EOPNOTSUPP;
		goto out;
	}
	nr_pages = rds_pages_in_vec(&args->vec);
	if (nr_pages == 0) {
		ret = -EINVAL;
		goto out;
	}
	if ((nr_pages - 1) > (RDS_MAX_MSG_SIZE >> PAGE_SHIFT)) {
		ret = -EMSGSIZE;
		goto out;
	}
	rdsdebug("RDS: get_mr addr %llx len %llu nr_pages %u\n",
		args->vec.addr, args->vec.bytes, nr_pages);
	pages = kcalloc(nr_pages, sizeof(struct page *), GFP_KERNEL);
	if (!pages) {
		ret = -ENOMEM;
		goto out;
	}
	mr = kzalloc(sizeof(struct rds_mr), GFP_KERNEL);
	if (!mr) {
		ret = -ENOMEM;
		goto out;
	}
	refcount_set(&mr->r_refcount, 1);
	RB_CLEAR_NODE(&mr->r_rb_node);
	mr->r_trans = rs->rs_transport;
	mr->r_sock = rs;
	if (args->flags & RDS_RDMA_USE_ONCE)
		mr->r_use_once = 1;
	if (args->flags & RDS_RDMA_INVALIDATE)
		mr->r_invalidate = 1;
	if (args->flags & RDS_RDMA_READWRITE)
		mr->r_write = 1;
	ret = rds_pin_pages(args->vec.addr, nr_pages, pages, 1);
	if (ret < 0)
		goto out;
	nents = ret;
	sg = kcalloc(nents, sizeof(*sg), GFP_KERNEL);
	if (!sg) {
		ret = -ENOMEM;
		goto out;
	}
	WARN_ON(!nents);
	sg_init_table(sg, nents);
	for (i = 0 ; i < nents; i++)
		sg_set_page(&sg[i], pages[i], PAGE_SIZE, 0);
	rdsdebug("RDS: trans_private nents is %u\n", nents);
	trans_private = rs->rs_transport->get_mr(sg, nents, rs,
						 &mr->r_key);
	if (IS_ERR(trans_private)) {
		for (i = 0 ; i < nents; i++)
			put_page(sg_page(&sg[i]));
		kfree(sg);
		ret = PTR_ERR(trans_private);
		goto out;
	}
	mr->r_trans_private = trans_private;
	rdsdebug("RDS: get_mr put_user key is %x cookie_addr %p\n",
	       mr->r_key, (void *)(unsigned long) args->cookie_addr);
	cookie = rds_rdma_make_cookie(mr->r_key, args->vec.addr & ~PAGE_MASK);
	if (cookie_ret)
		*cookie_ret = cookie;
	if (args->cookie_addr && put_user(cookie, (u64 __user *)(unsigned long) args->cookie_addr)) {
		ret = -EFAULT;
		goto out;
	}
	spin_lock_irqsave(&rs->rs_rdma_lock, flags);
	found = rds_mr_tree_walk(&rs->rs_rdma_keys, mr->r_key, mr);
	spin_unlock_irqrestore(&rs->rs_rdma_lock, flags);
	BUG_ON(found && found != mr);
	rdsdebug("RDS: get_mr key is %x\n", mr->r_key);
	if (mr_ret) {
		refcount_inc(&mr->r_refcount);
		*mr_ret = mr;
	}
	ret = 0;
out:
	kfree(pages);
	if (mr)
		rds_mr_put(mr);
	return ret;
}