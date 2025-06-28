static CURLcode smtp_connect(struct connectdata *conn,
                             bool *done)  
{
  CURLcode result;
  struct smtp_conn *smtpc = &conn->proto.smtpc;
  struct SessionHandle *data = conn->data;
  struct pingpong *pp = &smtpc->pp;
  const char *path = conn->data->state.path;
  int len;
  char localhost[HOSTNAME_MAX + 1];
  *done = FALSE;  
  Curl_reset_reqproto(conn);
  result = smtp_init(conn);
  if(CURLE_OK != result)
    return result;
  conn->bits.close = FALSE;
  pp->response_time = RESP_TIMEOUT;  
  pp->statemach_act = smtp_statemach_act;
  pp->endofresp = smtp_endofresp;
  pp->conn = conn;
  if(conn->bits.tunnel_proxy && conn->bits.httpproxy) {
    struct HTTP http_proxy;
    struct FTP *smtp_save;
    smtp_save = data->state.proto.smtp;
    memset(&http_proxy, 0, sizeof(http_proxy));
    data->state.proto.http = &http_proxy;
    result = Curl_proxyCONNECT(conn, FIRSTSOCKET,
                               conn->host.name, conn->remote_port);
    data->state.proto.smtp = smtp_save;
    if(CURLE_OK != result)
      return result;
  }
  if((conn->handler->protocol & CURLPROTO_SMTPS) &&
      data->state.used_interface != Curl_if_multi) {
    result = Curl_ssl_connect(conn, FIRSTSOCKET);
    if(result)
      return result;
  }
  Curl_pp_init(pp);  
  pp->response_time = RESP_TIMEOUT;  
  pp->statemach_act = smtp_statemach_act;
  pp->endofresp = smtp_endofresp;
  pp->conn = conn;
  if(!*path) {
    if(!Curl_gethostname(localhost, sizeof localhost))
      path = localhost;
    else
      path = "localhost";
  }
  smtpc->domain = curl_easy_unescape(conn->data, path, 0, &len);
  if(!smtpc->domain)
    return CURLE_OUT_OF_MEMORY;
  state(conn, SMTP_SERVERGREET);
  if(data->state.used_interface == Curl_if_multi)
    result = smtp_multi_statemach(conn, done);
  else {
    result = smtp_easy_statemach(conn);
    if(!result)
      *done = TRUE;
  }
  return result;
}