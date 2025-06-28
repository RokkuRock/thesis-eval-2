static int pop_fetch_headers (CONTEXT *ctx)
{
  int i, ret, old_count, new_count, deleted;
  unsigned short hcached = 0, bcached;
  POP_DATA *pop_data = (POP_DATA *)ctx->data;
  progress_t progress;
#ifdef USE_HCACHE
  header_cache_t *hc = NULL;
  void *data;
  hc = pop_hcache_open (pop_data, ctx->path);
#endif
  time (&pop_data->check_time);
  pop_data->clear_cache = 0;
  for (i = 0; i < ctx->msgcount; i++)
    ctx->hdrs[i]->refno = -1;
  old_count = ctx->msgcount;
  ret = pop_fetch_data (pop_data, "UIDL\r\n", NULL, fetch_uidl, ctx);
  new_count = ctx->msgcount;
  ctx->msgcount = old_count;
  if (pop_data->cmd_uidl == 2)
  {
    if (ret == 0)
    {
      pop_data->cmd_uidl = 1;
      dprint (1, (debugfile, "pop_fetch_headers: set UIDL capability\n"));
    }
    if (ret == -2 && pop_data->cmd_uidl == 2)
    {
      pop_data->cmd_uidl = 0;
      dprint (1, (debugfile, "pop_fetch_headers: unset UIDL capability\n"));
      snprintf (pop_data->err_msg, sizeof (pop_data->err_msg), "%s",
	      _("Command UIDL is not supported by server."));
    }
  }
  if (!ctx->quiet)
    mutt_progress_init (&progress, _("Fetching message headers..."),
                        MUTT_PROGRESS_MSG, ReadInc, new_count - old_count);
  if (ret == 0)
  {
    for (i = 0, deleted = 0; i < old_count; i++)
    {
      if (ctx->hdrs[i]->refno == -1)
      {
	ctx->hdrs[i]->deleted = 1;
	deleted++;
      }
    }
    if (deleted > 0)
    {
      mutt_error (_("%d messages have been lost. Try reopening the mailbox."),
		  deleted);
      mutt_sleep (2);
    }
    for (i = old_count; i < new_count; i++)
    {
      if (!ctx->quiet)
	mutt_progress_update (&progress, i + 1 - old_count, -1);
#if USE_HCACHE
      if ((data = mutt_hcache_fetch (hc, ctx->hdrs[i]->data, strlen)))
      {
	char *uidl = safe_strdup (ctx->hdrs[i]->data);
	int refno = ctx->hdrs[i]->refno;
	int index = ctx->hdrs[i]->index;
	HEADER *h = mutt_hcache_restore ((unsigned char *) data, NULL);
	mutt_free_header (&ctx->hdrs[i]);
	ctx->hdrs[i] = h;
	ctx->hdrs[i]->refno = refno;
	ctx->hdrs[i]->index = index;
	ctx->hdrs[i]->data = uidl;
	ret = 0;
	hcached = 1;
      }
      else
#endif
      if ((ret = pop_read_header (pop_data, ctx->hdrs[i])) < 0)
	break;
#if USE_HCACHE
      else
      {
	mutt_hcache_store (hc, ctx->hdrs[i]->data, ctx->hdrs[i], 0, strlen, MUTT_GENERATE_UIDVALIDITY);
      }
      mutt_hcache_free (&data);
#endif
      bcached = mutt_bcache_exists (pop_data->bcache, ctx->hdrs[i]->data) == 0;
      ctx->hdrs[i]->old = 0;
      ctx->hdrs[i]->read = 0;
      if (hcached)
      {
        if (bcached)
          ctx->hdrs[i]->read = 1;
        else if (option (OPTMARKOLD))
          ctx->hdrs[i]->old = 1;
      }
      else
      {
        if (bcached)
          ctx->hdrs[i]->read = 1;
      }
      ctx->msgcount++;
    }
    if (i > old_count)
      mx_update_context (ctx, i - old_count);
  }
#if USE_HCACHE
    mutt_hcache_close (hc);
#endif
  if (ret < 0)
  {
    for (i = ctx->msgcount; i < new_count; i++)
      mutt_free_header (&ctx->hdrs[i]);
    return ret;
  }
  if (option (OPTMESSAGECACHECLEAN))
    mutt_bcache_list (pop_data->bcache, msg_cache_check, (void*)ctx);
  mutt_clear_error ();
  return (new_count - old_count);
}