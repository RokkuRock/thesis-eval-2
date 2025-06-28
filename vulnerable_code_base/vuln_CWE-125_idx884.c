static noinline int smb2_write_pipe(struct ksmbd_work *work)
{
	struct smb2_write_req *req = smb2_get_msg(work->request_buf);
	struct smb2_write_rsp *rsp = smb2_get_msg(work->response_buf);
	struct ksmbd_rpc_command *rpc_resp;
	u64 id = 0;
	int err = 0, ret = 0;
	char *data_buf;
	size_t length;
	length = le32_to_cpu(req->Length);
	id = req->VolatileFileId;
	if (le16_to_cpu(req->DataOffset) ==
	    offsetof(struct smb2_write_req, Buffer)) {
		data_buf = (char *)&req->Buffer[0];
	} else {
		if ((u64)le16_to_cpu(req->DataOffset) + length >
		    get_rfc1002_len(work->request_buf)) {
			pr_err("invalid write data offset %u, smb_len %u\n",
			       le16_to_cpu(req->DataOffset),
			       get_rfc1002_len(work->request_buf));
			err = -EINVAL;
			goto out;
		}
		data_buf = (char *)(((char *)&req->hdr.ProtocolId) +
				le16_to_cpu(req->DataOffset));
	}
	rpc_resp = ksmbd_rpc_write(work->sess, id, data_buf, length);
	if (rpc_resp) {
		if (rpc_resp->flags == KSMBD_RPC_ENOTIMPLEMENTED) {
			rsp->hdr.Status = STATUS_NOT_SUPPORTED;
			kvfree(rpc_resp);
			smb2_set_err_rsp(work);
			return -EOPNOTSUPP;
		}
		if (rpc_resp->flags != KSMBD_RPC_OK) {
			rsp->hdr.Status = STATUS_INVALID_HANDLE;
			smb2_set_err_rsp(work);
			kvfree(rpc_resp);
			return ret;
		}
		kvfree(rpc_resp);
	}
	rsp->StructureSize = cpu_to_le16(17);
	rsp->DataOffset = 0;
	rsp->Reserved = 0;
	rsp->DataLength = cpu_to_le32(length);
	rsp->DataRemaining = 0;
	rsp->Reserved2 = 0;
	inc_rfc1001_len(work->response_buf, 16);
	return 0;
out:
	if (err) {
		rsp->hdr.Status = STATUS_INVALID_HANDLE;
		smb2_set_err_rsp(work);
	}
	return err;
}