CURLcode Curl_close(struct Curl_easy *data)
{
  struct Curl_multi *m;
  if(!data)
    return CURLE_OK;
  Curl_expire_clear(data);  
  m = data->multi;
  if(m)
    curl_multi_remove_handle(data->multi, data);
  if(data->multi_easy)
    curl_multi_cleanup(data->multi_easy);
  Curl_llist_destroy(&data->state.timeoutlist, NULL);
  data->magic = 0;  
  if(data->state.rangestringalloc)
    free(data->state.range);
  Curl_free_request_state(data);
  Curl_ssl_close_all(data);
  Curl_safefree(data->state.first_host);
  Curl_safefree(data->state.scratch);
  Curl_ssl_free_certinfo(data);
  free(data->req.newurl);
  data->req.newurl = NULL;
  if(data->change.referer_alloc) {
    Curl_safefree(data->change.referer);
    data->change.referer_alloc = FALSE;
  }
  data->change.referer = NULL;
  Curl_up_free(data);
  Curl_safefree(data->state.buffer);
  Curl_safefree(data->state.headerbuff);
  Curl_safefree(data->state.ulbuf);
  Curl_flush_cookies(data, 1);
  Curl_digest_cleanup(data);
  Curl_safefree(data->info.contenttype);
  Curl_safefree(data->info.wouldredirect);
  Curl_resolver_cleanup(data->state.resolver);
  Curl_http2_cleanup_dependencies(data);
  Curl_convert_close(data);
  if(data->share) {
    Curl_share_lock(data, CURL_LOCK_DATA_SHARE, CURL_LOCK_ACCESS_SINGLE);
    data->share->dirty--;
    Curl_share_unlock(data, CURL_LOCK_DATA_SHARE);
  }
  Curl_wildcard_dtor(&data->wildcard);
  Curl_freeset(data);
  free(data);
  return CURLE_OK;
}