static CURLcode pop3_parse_url_path(struct connectdata *conn)
{
  struct pop3_conn *pop3c = &conn->proto.pop3c;
  struct SessionHandle *data = conn->data;
  const char *path = data->state.path;
  pop3c->mailbox = curl_easy_unescape(data, path, 0, NULL);
  if(!pop3c->mailbox)
    return CURLE_OUT_OF_MEMORY;
  return CURLE_OK;
}