static int audit_log_single_execve_arg(struct audit_context *context,
					struct audit_buffer **ab,
					int arg_num,
					size_t *len_sent,
					const char __user *p,
					char *buf)
{
	char arg_num_len_buf[12];
	const char __user *tmp_p = p;
	size_t arg_num_len = snprintf(arg_num_len_buf, 12, "%d", arg_num) + 5;
	size_t len, len_left, to_send;
	size_t max_execve_audit_len = MAX_EXECVE_AUDIT_LEN;
	unsigned int i, has_cntl = 0, too_long = 0;
	int ret;
	len_left = len = strnlen_user(p, MAX_ARG_STRLEN) - 1;
	if (WARN_ON_ONCE(len < 0 || len > MAX_ARG_STRLEN - 1)) {
		send_sig(SIGKILL, current, 0);
		return -1;
	}
	do {
		if (len_left > MAX_EXECVE_AUDIT_LEN)
			to_send = MAX_EXECVE_AUDIT_LEN;
		else
			to_send = len_left;
		ret = copy_from_user(buf, tmp_p, to_send);
		if (ret) {
			WARN_ON(1);
			send_sig(SIGKILL, current, 0);
			return -1;
		}
		buf[to_send] = '\0';
		has_cntl = audit_string_contains_control(buf, to_send);
		if (has_cntl) {
			max_execve_audit_len = MAX_EXECVE_AUDIT_LEN / 2;
			break;
		}
		len_left -= to_send;
		tmp_p += to_send;
	} while (len_left > 0);
	len_left = len;
	if (len > max_execve_audit_len)
		too_long = 1;
	for (i = 0; len_left > 0; i++) {
		int room_left;
		if (len_left > max_execve_audit_len)
			to_send = max_execve_audit_len;
		else
			to_send = len_left;
		room_left = MAX_EXECVE_AUDIT_LEN - arg_num_len - *len_sent;
		if (has_cntl)
			room_left -= (to_send * 2);
		else
			room_left -= to_send;
		if (room_left < 0) {
			*len_sent = 0;
			audit_log_end(*ab);
			*ab = audit_log_start(context, GFP_KERNEL, AUDIT_EXECVE);
			if (!*ab)
				return 0;
		}
		if ((i == 0) && (too_long))
			audit_log_format(*ab, " a%d_len=%zu", arg_num,
					 has_cntl ? 2*len : len);
		if (len >= max_execve_audit_len)
			ret = copy_from_user(buf, p, to_send);
		else
			ret = 0;
		if (ret) {
			WARN_ON(1);
			send_sig(SIGKILL, current, 0);
			return -1;
		}
		buf[to_send] = '\0';
		audit_log_format(*ab, " a%d", arg_num);
		if (too_long)
			audit_log_format(*ab, "[%d]", i);
		audit_log_format(*ab, "=");
		if (has_cntl)
			audit_log_n_hex(*ab, buf, to_send);
		else
			audit_log_string(*ab, buf);
		p += to_send;
		len_left -= to_send;
		*len_sent += arg_num_len;
		if (has_cntl)
			*len_sent += to_send * 2;
		else
			*len_sent += to_send;
	}
	return len + 1;
}