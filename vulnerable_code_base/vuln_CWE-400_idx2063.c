read_packet(int fd, gss_buffer_t buf, int timeout, int first)
{
	int	  ret;
	static uint32_t		len = 0;
	static char		len_buf[4];
	static int		len_buf_pos = 0;
	static char *		tmpbuf = 0;
	static int		tmpbuf_pos = 0;
	if (first) {
		len_buf_pos = 0;
		return -2;
	}
	if (len_buf_pos < 4) {
		ret = timed_read(fd, &len_buf[len_buf_pos], 4 - len_buf_pos,
		    timeout);
		if (ret == -1) {
			if (errno == EINTR || errno == EAGAIN)
				return -2;
			LOG(LOG_ERR, ("%s", strerror(errno)));
			return -1;
		}
		if (ret == 0) {		 
			if (len_buf_pos == 0)
				return 0;
			LOG(LOG_INFO, ("EOF reading packet len"));
			return -1;
		}
		len_buf_pos += ret;
	}
	if (len_buf_pos != 4)
		return -2;
	len = ntohl(*(uint32_t *)len_buf);
	if (len > GSTD_MAXPACKETCONTENTS + 512) {
		LOG(LOG_ERR, ("ridiculous length, %ld", len));
		return -1;
	}
	if (!tmpbuf) {
		if ((tmpbuf = malloc(len)) == NULL) {
			LOG(LOG_CRIT, ("malloc failure, %ld bytes", len));
			return -1;
		}
	}
	ret = timed_read(fd, tmpbuf + tmpbuf_pos, len - tmpbuf_pos, timeout);
	if (ret == -1) {
		if (errno == EINTR || errno == EAGAIN)
			return -2;
		LOG(LOG_ERR, ("%s", strerror(errno)));
		return -1;
	}
	if (ret == 0) {
		LOG(LOG_ERR, ("EOF while reading packet (len=%d)", len));
		return -1;
	}
	tmpbuf_pos += ret;
	if (tmpbuf_pos == len) {
		buf->length = len;
		buf->value = tmpbuf;
		len = len_buf_pos = tmpbuf_pos = 0;
		tmpbuf = NULL;
		LOG(LOG_DEBUG, ("read packet of length %d", buf->length));
		return 1;
	}
	return -2;
}