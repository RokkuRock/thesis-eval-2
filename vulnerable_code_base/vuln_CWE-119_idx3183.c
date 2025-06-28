static int msg_cache_check (const char *id, body_cache_t *bcache, void *data)
{
  CONTEXT *ctx;
  POP_DATA *pop_data;
  int i;
  if (!(ctx = (CONTEXT *)data))
    return -1;
  if (!(pop_data = (POP_DATA *)ctx->data))
    return -1;
#ifdef USE_HCACHE
  if (strcmp (HC_FNAME "." HC_FEXT, id) == 0)
    return 0;
#endif
  for (i = 0; i < ctx->msgcount; i++)
    if (ctx->hdrs[i]->data && mutt_strcmp (ctx->hdrs[i]->data, id) == 0)
      return 0;
  return mutt_bcache_del (bcache, id);
}