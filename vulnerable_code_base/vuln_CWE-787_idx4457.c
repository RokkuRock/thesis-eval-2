CURLcode Curl_smtp_escape_eob(struct connectdata *conn, const ssize_t nread)
{
  ssize_t i;
  ssize_t si;
  struct Curl_easy *data = conn->data;
  struct SMTP *smtp = data->req.protop;
  char *scratch = data->state.scratch;
  char *newscratch = NULL;
  char *oldscratch = NULL;
  size_t eob_sent;
  if(!scratch || data->set.crlf) {
    oldscratch = scratch;
    scratch = newscratch = malloc(2 * data->set.buffer_size);
    if(!newscratch) {
      failf(data, "Failed to alloc scratch buffer!");
      return CURLE_OUT_OF_MEMORY;
    }
  }
  eob_sent = smtp->eob;
  for(i = 0, si = 0; i < nread; i++) {
    if(SMTP_EOB[smtp->eob] == data->req.upload_fromhere[i]) {
      smtp->eob++;
      if(2 == smtp->eob || SMTP_EOB_LEN == smtp->eob)
        smtp->trailing_crlf = TRUE;
      else
        smtp->trailing_crlf = FALSE;
    }
    else if(smtp->eob) {
      memcpy(&scratch[si], &SMTP_EOB[eob_sent], smtp->eob - eob_sent);
      si += smtp->eob - eob_sent;
      if(SMTP_EOB[0] == data->req.upload_fromhere[i])
        smtp->eob = 1;
      else
        smtp->eob = 0;
      eob_sent = 0;
      smtp->trailing_crlf = FALSE;
    }
    if(SMTP_EOB_FIND_LEN == smtp->eob) {
      memcpy(&scratch[si], &SMTP_EOB_REPL[eob_sent],
             SMTP_EOB_REPL_LEN - eob_sent);
      si += SMTP_EOB_REPL_LEN - eob_sent;
      smtp->eob = 0;
      eob_sent = 0;
    }
    else if(!smtp->eob)
      scratch[si++] = data->req.upload_fromhere[i];
  }
  if(smtp->eob - eob_sent) {
    memcpy(&scratch[si], &SMTP_EOB[eob_sent], smtp->eob - eob_sent);
    si += smtp->eob - eob_sent;
  }
  if(si != nread) {
    data->req.upload_fromhere = scratch;
    data->state.scratch = scratch;
    free(oldscratch);
    data->req.upload_present = si;
  }
  else
    free(newscratch);
  return CURLE_OK;
}