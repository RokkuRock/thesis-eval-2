static void http_manage_server_side_cookies(struct stream *s, struct channel *res)
{
	struct session *sess = s->sess;
	struct http_txn *txn = s->txn;
	struct htx *htx;
	struct http_hdr_ctx ctx;
	struct server *srv;
	char *hdr_beg, *hdr_end;
	char *prev, *att_beg, *att_end, *equal, *val_beg, *val_end, *next;
	int is_cookie2 = 0;
	htx = htxbuf(&res->buf);
	ctx.blk = NULL;
	while (1) {
		int is_first = 1;
		if (!http_find_header(htx, ist("Set-Cookie"), &ctx, 1)) {
			if (!http_find_header(htx, ist("Set-Cookie2"), &ctx, 1))
				break;
			is_cookie2 = 1;
		}
		txn->flags |= TX_SCK_PRESENT;
		if (s->be->cookie_name == NULL && sess->fe->capture_name == NULL)
			break;
		hdr_beg = ctx.value.ptr;
		hdr_end = hdr_beg + ctx.value.len;
		for (prev = hdr_beg; prev < hdr_end; prev = next) {
			att_beg = prev;
			if (!is_first)
				att_beg++;
			is_first = 0;
			while (att_beg < hdr_end && HTTP_IS_SPHT(*att_beg))
				att_beg++;
			equal = att_end = att_beg;
			while (equal < hdr_end) {
				if (*equal == '=' || *equal == ';' || (is_cookie2 && *equal == ','))
					break;
				if (HTTP_IS_SPHT(*equal++))
					continue;
				att_end = equal;
			}
			if (equal < hdr_end && *equal == '=') {
				val_beg = equal + 1;
				while (val_beg < hdr_end && HTTP_IS_SPHT(*val_beg))
					val_beg++;
				next = http_find_cookie_value_end(val_beg, hdr_end);
				val_end = next;
				while (val_end > val_beg && HTTP_IS_SPHT(*(val_end - 1)))
					val_end--;
			}
			else {
				val_beg = val_end = next = equal;
			}
			if (next < hdr_end) {
				if (is_cookie2)
					next = http_find_hdr_value_end(next, hdr_end);
				else
					next = hdr_end;
			}
			if (equal == val_end)
				continue;
			if (unlikely(att_end != equal || val_beg > equal + 1)) {
				int stripped_before = 0;
				int stripped_after = 0;
				if (att_end != equal) {
					memmove(att_end, equal, hdr_end - equal);
					stripped_before = (att_end - equal);
					equal   += stripped_before;
					val_beg += stripped_before;
				}
				if (val_beg > equal + 1) {
					memmove(equal + 1, val_beg, hdr_end + stripped_before - val_beg);
					stripped_after = (equal + 1) - val_beg;
					val_beg += stripped_after;
					stripped_before += stripped_after;
				}
				val_end      += stripped_before;
				next         += stripped_before;
				hdr_end      += stripped_before;
				htx_change_blk_value_len(htx, ctx.blk, hdr_end - hdr_beg);
				ctx.value.len = hdr_end - hdr_beg;
			}
			if (sess->fe->capture_name != NULL &&
			    txn->srv_cookie == NULL &&
			    (val_end - att_beg >= sess->fe->capture_namelen) &&
			    memcmp(att_beg, sess->fe->capture_name, sess->fe->capture_namelen) == 0) {
				int log_len = val_end - att_beg;
				if ((txn->srv_cookie = pool_alloc(pool_head_capture)) == NULL) {
					ha_alert("HTTP logging : out of memory.\n");
				}
				else {
					if (log_len > sess->fe->capture_len)
						log_len = sess->fe->capture_len;
					memcpy(txn->srv_cookie, att_beg, log_len);
					txn->srv_cookie[log_len] = 0;
				}
			}
			srv = objt_server(s->target);
			if (!(s->flags & SF_IGNORE_PRST) &&
			    (att_end - att_beg == s->be->cookie_len) && (s->be->cookie_name != NULL) &&
			    (memcmp(att_beg, s->be->cookie_name, att_end - att_beg) == 0)) {
				txn->flags &= ~TX_SCK_MASK;
				txn->flags |= TX_SCK_FOUND;
				if (s->be->ck_opts & PR_CK_PSV) {
				}
				else if ((srv && (s->be->ck_opts & PR_CK_INS)) ||
				    ((s->flags & SF_DIRECT) && (s->be->ck_opts & PR_CK_IND))) {
					if (prev == hdr_beg && next == hdr_end) {
						http_remove_header(htx, &ctx);
					} else {
						int delta = http_del_hdr_value(hdr_beg, hdr_end, &prev, next);
						next      = prev;
						hdr_end  += delta;
					}
					txn->flags &= ~TX_SCK_MASK;
					txn->flags |= TX_SCK_DELETED;
				}
				else if (srv && srv->cookie && (s->be->ck_opts & PR_CK_RW)) {
					int sliding, delta;
					ctx.value = ist2(val_beg, val_end - val_beg);
				        ctx.lws_before = ctx.lws_after = 0;
					http_replace_header_value(htx, &ctx, ist2(srv->cookie, srv->cklen));
					delta     = srv->cklen - (val_end - val_beg);
					sliding   = (ctx.value.ptr - val_beg);
					hdr_beg  += sliding;
					val_beg  += sliding;
					next     += sliding + delta;
					hdr_end  += sliding + delta;
					txn->flags &= ~TX_SCK_MASK;
					txn->flags |= TX_SCK_REPLACED;
				}
				else if (srv && srv->cookie && (s->be->ck_opts & PR_CK_PFX)) {
					int sliding, delta;
					ctx.value = ist2(val_beg, 0);
				        ctx.lws_before = ctx.lws_after = 0;
					http_replace_header_value(htx, &ctx, ist2(srv->cookie, srv->cklen + 1));
					delta     = srv->cklen + 1;
					sliding   = (ctx.value.ptr - val_beg);
					hdr_beg  += sliding;
					val_beg  += sliding;
					next     += sliding + delta;
					hdr_end  += sliding + delta;
					val_beg[srv->cklen] = COOKIE_DELIM;
					txn->flags &= ~TX_SCK_MASK;
					txn->flags |= TX_SCK_REPLACED;
				}
			}
		}
	}
}