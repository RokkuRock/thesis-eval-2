static int parse_report(transport_smart *transport, git_push *push)
{
	git_pkt *pkt = NULL;
	const char *line_end = NULL;
	gitno_buffer *buf = &transport->buffer;
	int error, recvd;
	git_buf data_pkt_buf = GIT_BUF_INIT;
	for (;;) {
		if (buf->offset > 0)
			error = git_pkt_parse_line(&pkt, buf->data,
						   &line_end, buf->offset);
		else
			error = GIT_EBUFS;
		if (error < 0 && error != GIT_EBUFS) {
			error = -1;
			goto done;
		}
		if (error == GIT_EBUFS) {
			if ((recvd = gitno_recv(buf)) < 0) {
				error = recvd;
				goto done;
			}
			if (recvd == 0) {
				giterr_set(GITERR_NET, "early EOF");
				error = GIT_EEOF;
				goto done;
			}
			continue;
		}
		gitno_consume(buf, line_end);
		error = 0;
		if (pkt == NULL)
			continue;
		switch (pkt->type) {
			case GIT_PKT_DATA:
				error = add_push_report_sideband_pkt(push, (git_pkt_data *)pkt, &data_pkt_buf);
				break;
			case GIT_PKT_ERR:
				giterr_set(GITERR_NET, "report-status: Error reported: %s",
					((git_pkt_err *)pkt)->error);
				error = -1;
				break;
			case GIT_PKT_PROGRESS:
				if (transport->progress_cb) {
					git_pkt_progress *p = (git_pkt_progress *) pkt;
					error = transport->progress_cb(p->data, p->len, transport->message_cb_payload);
				}
				break;
			default:
				error = add_push_report_pkt(push, pkt);
				break;
		}
		git_pkt_free(pkt);
		if (error == GIT_ITEROVER) {
			error = 0;
			if (data_pkt_buf.size > 0) {
				giterr_set(GITERR_NET, "Incomplete pack data pkt-line");
				error = GIT_ERROR;
			}
			goto done;
		}
		if (error < 0) {
			goto done;
		}
	}
done:
	git_buf_free(&data_pkt_buf);
	return error;
}