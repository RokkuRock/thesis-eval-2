int rds_sendmsg(struct socket *sock, struct msghdr *msg, size_t payload_len)
{
	struct sock *sk = sock->sk;
	struct rds_sock *rs = rds_sk_to_rs(sk);
	DECLARE_SOCKADDR(struct sockaddr_in *, usin, msg->msg_name);
	__be32 daddr;
	__be16 dport;
	struct rds_message *rm = NULL;
	struct rds_connection *conn;
	int ret = 0;
	int queued = 0, allocated_mr = 0;
	int nonblock = msg->msg_flags & MSG_DONTWAIT;
	long timeo = sock_sndtimeo(sk, nonblock);
	if (msg->msg_flags & ~(MSG_DONTWAIT | MSG_CMSG_COMPAT)) {
		ret = -EOPNOTSUPP;
		goto out;
	}
	if (msg->msg_namelen) {
		if (msg->msg_namelen < sizeof(*usin) || usin->sin_family != AF_INET) {
			ret = -EINVAL;
			goto out;
		}
		daddr = usin->sin_addr.s_addr;
		dport = usin->sin_port;
	} else {
		lock_sock(sk);
		daddr = rs->rs_conn_addr;
		dport = rs->rs_conn_port;
		release_sock(sk);
	}
	if (daddr == 0 || rs->rs_bound_addr == 0) {
		ret = -ENOTCONN;  
		goto out;
	}
	if (payload_len > rds_sk_sndbuf(rs)) {
		ret = -EMSGSIZE;
		goto out;
	}
	ret = rds_rm_size(msg, payload_len);
	if (ret < 0)
		goto out;
	rm = rds_message_alloc(ret, GFP_KERNEL);
	if (!rm) {
		ret = -ENOMEM;
		goto out;
	}
	if (payload_len) {
		rm->data.op_sg = rds_message_alloc_sgs(rm, ceil(payload_len, PAGE_SIZE));
		if (!rm->data.op_sg) {
			ret = -ENOMEM;
			goto out;
		}
		ret = rds_message_copy_from_user(rm, &msg->msg_iter);
		if (ret)
			goto out;
	}
	rm->data.op_active = 1;
	rm->m_daddr = daddr;
	if (rs->rs_conn && rs->rs_conn->c_faddr == daddr)
		conn = rs->rs_conn;
	else {
		conn = rds_conn_create_outgoing(sock_net(sock->sk),
						rs->rs_bound_addr, daddr,
					rs->rs_transport,
					sock->sk->sk_allocation);
		if (IS_ERR(conn)) {
			ret = PTR_ERR(conn);
			goto out;
		}
		rs->rs_conn = conn;
	}
	ret = rds_cmsg_send(rs, rm, msg, &allocated_mr);
	if (ret)
		goto out;
	if (rm->rdma.op_active && !conn->c_trans->xmit_rdma) {
		printk_ratelimited(KERN_NOTICE "rdma_op %p conn xmit_rdma %p\n",
			       &rm->rdma, conn->c_trans->xmit_rdma);
		ret = -EOPNOTSUPP;
		goto out;
	}
	if (rm->atomic.op_active && !conn->c_trans->xmit_atomic) {
		printk_ratelimited(KERN_NOTICE "atomic_op %p conn xmit_atomic %p\n",
			       &rm->atomic, conn->c_trans->xmit_atomic);
		ret = -EOPNOTSUPP;
		goto out;
	}
	rds_conn_connect_if_down(conn);
	ret = rds_cong_wait(conn->c_fcong, dport, nonblock, rs);
	if (ret) {
		rs->rs_seen_congestion = 1;
		goto out;
	}
	while (!rds_send_queue_rm(rs, conn, rm, rs->rs_bound_port,
				  dport, &queued)) {
		rds_stats_inc(s_send_queue_full);
		if (nonblock) {
			ret = -EAGAIN;
			goto out;
		}
		timeo = wait_event_interruptible_timeout(*sk_sleep(sk),
					rds_send_queue_rm(rs, conn, rm,
							  rs->rs_bound_port,
							  dport,
							  &queued),
					timeo);
		rdsdebug("sendmsg woke queued %d timeo %ld\n", queued, timeo);
		if (timeo > 0 || timeo == MAX_SCHEDULE_TIMEOUT)
			continue;
		ret = timeo;
		if (ret == 0)
			ret = -ETIMEDOUT;
		goto out;
	}
	rds_stats_inc(s_send_queued);
	ret = rds_send_xmit(conn);
	if (ret == -ENOMEM || ret == -EAGAIN)
		queue_delayed_work(rds_wq, &conn->c_send_w, 1);
	rds_message_put(rm);
	return payload_len;
out:
	if (allocated_mr)
		rds_rdma_unuse(rs, rds_rdma_cookie_key(rm->m_rdma_cookie), 1);
	if (rm)
		rds_message_put(rm);
	return ret;
}