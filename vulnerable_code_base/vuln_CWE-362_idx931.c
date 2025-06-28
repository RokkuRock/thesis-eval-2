smb_send_kvec(struct TCP_Server_Info *server, struct kvec *iov, size_t n_vec,
		size_t *sent)
{
	int rc = 0;
	int i = 0;
	struct msghdr smb_msg;
	unsigned int remaining;
	size_t first_vec = 0;
	struct socket *ssocket = server->ssocket;
	*sent = 0;
	if (ssocket == NULL)
		return -ENOTSOCK;  
	smb_msg.msg_name = (struct sockaddr *) &server->dstaddr;
	smb_msg.msg_namelen = sizeof(struct sockaddr);
	smb_msg.msg_control = NULL;
	smb_msg.msg_controllen = 0;
	if (server->noblocksnd)
		smb_msg.msg_flags = MSG_DONTWAIT + MSG_NOSIGNAL;
	else
		smb_msg.msg_flags = MSG_NOSIGNAL;
	remaining = 0;
	for (i = 0; i < n_vec; i++)
		remaining += iov[i].iov_len;
	i = 0;
	while (remaining) {
		rc = kernel_sendmsg(ssocket, &smb_msg, &iov[first_vec],
				    n_vec - first_vec, remaining);
		if (rc == -ENOSPC || rc == -EAGAIN) {
			WARN_ON_ONCE(rc == -ENOSPC);
			i++;
			if (i >= 14 || (!server->noblocksnd && (i > 2))) {
				cERROR(1, "sends on sock %p stuck for 15 "
					  "seconds", ssocket);
				rc = -EAGAIN;
				break;
			}
			msleep(1 << i);
			continue;
		}
		if (rc < 0)
			break;
		*sent += rc;
		if (rc == remaining) {
			remaining = 0;
			break;
		}
		if (rc > remaining) {
			cERROR(1, "sent %d requested %d", rc, remaining);
			break;
		}
		if (rc == 0) {
			cERROR(1, "tcp sent no data");
			msleep(500);
			continue;
		}
		remaining -= rc;
		for (i = first_vec; i < n_vec; i++) {
			if (iov[i].iov_len) {
				if (rc > iov[i].iov_len) {
					rc -= iov[i].iov_len;
					iov[i].iov_len = 0;
				} else {
					iov[i].iov_base += rc;
					iov[i].iov_len -= rc;
					first_vec = i;
					break;
				}
			}
		}
		i = 0;  
		rc = 0;
	}
	return rc;
}