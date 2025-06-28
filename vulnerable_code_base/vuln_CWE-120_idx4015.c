imap_auth_res_t imap_auth_cram_md5 (IMAP_DATA* idata, const char* method)
{
  char ibuf[LONG_STRING*2], obuf[LONG_STRING];
  unsigned char hmac_response[MD5_DIGEST_LEN];
  int len;
  int rc;
  if (!mutt_bit_isset (idata->capabilities, ACRAM_MD5))
    return IMAP_AUTH_UNAVAIL;
  mutt_message _("Authenticating (CRAM-MD5)...");
  if (mutt_account_getlogin (&idata->conn->account))
    return IMAP_AUTH_FAILURE;
  if (mutt_account_getpass (&idata->conn->account))
    return IMAP_AUTH_FAILURE;
  imap_cmd_start (idata, "AUTHENTICATE CRAM-MD5");
  do
    rc = imap_cmd_step (idata);
  while (rc == IMAP_CMD_CONTINUE);
  if (rc != IMAP_CMD_RESPOND)
  {
    dprint (1, (debugfile, "Invalid response from server: %s\n", ibuf));
    goto bail;
  }
  if ((len = mutt_from_base64 (obuf, idata->buf + 2)) == -1)
  {
    dprint (1, (debugfile, "Error decoding base64 response.\n"));
    goto bail;
  }
  obuf[len] = '\0';
  dprint (2, (debugfile, "CRAM challenge: %s\n", obuf));
  hmac_md5 (idata->conn->account.pass, obuf, hmac_response);
  snprintf (obuf, sizeof (obuf),
    "%s %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
    idata->conn->account.user,
    hmac_response[0], hmac_response[1], hmac_response[2], hmac_response[3],
    hmac_response[4], hmac_response[5], hmac_response[6], hmac_response[7],
    hmac_response[8], hmac_response[9], hmac_response[10], hmac_response[11],
    hmac_response[12], hmac_response[13], hmac_response[14], hmac_response[15]);
  dprint(2, (debugfile, "CRAM response: %s\n", obuf));
  mutt_to_base64 ((unsigned char*) ibuf, (unsigned char*) obuf, strlen (obuf),
		  sizeof (ibuf) - 2);
  safe_strcat (ibuf, sizeof (ibuf), "\r\n");
  mutt_socket_write (idata->conn, ibuf);
  do
    rc = imap_cmd_step (idata);
  while (rc == IMAP_CMD_CONTINUE);
  if (rc != IMAP_CMD_OK)
  {
    dprint (1, (debugfile, "Error receiving server response.\n"));
    goto bail;
  }
  if (imap_code (idata->buf))
    return IMAP_AUTH_SUCCESS;
 bail:
  mutt_error _("CRAM-MD5 authentication failed.");
  mutt_sleep (2);
  return IMAP_AUTH_FAILURE;
}