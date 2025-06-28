uint32_t sftp_parse_handle(struct sftpjob *job, struct handleid *id) {
  uint32_t len, rc;
  if((rc = sftp_parse_uint32(job, &len)) != SSH_FX_OK || len != 8 ||
     (rc = sftp_parse_uint32(job, &id->id)) != SSH_FX_OK ||
     (rc = sftp_parse_uint32(job, &id->tag) != SSH_FX_OK))
    return rc;
  return SSH_FX_OK;
}