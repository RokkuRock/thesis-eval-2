int io_msg_ring(struct io_kiocb *req, unsigned int issue_flags)
{
	struct io_msg *msg = io_kiocb_to_cmd(req, struct io_msg);
	int ret;
	ret = -EBADFD;
	if (!io_is_uring_fops(req->file))
		goto done;
	switch (msg->cmd) {
	case IORING_MSG_DATA:
		ret = io_msg_ring_data(req);
		break;
	case IORING_MSG_SEND_FD:
		ret = io_msg_send_fd(req, issue_flags);
		break;
	default:
		ret = -EINVAL;
		break;
	}
done:
	if (ret < 0)
		req_set_fail(req);
	io_req_set_res(req, ret, 0);
	io_put_file(req->file);
	req->file = NULL;
	return IOU_OK;
}