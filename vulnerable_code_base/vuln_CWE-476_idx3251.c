smb2_flush(smb_request_t *sr)
{
	smb_ofile_t *of = NULL;
	uint16_t StructSize;
	uint16_t reserved1;
	uint32_t reserved2;
	smb2fid_t smb2fid;
	uint32_t status;
	int rc = 0;
	rc = smb_mbc_decodef(
	    &sr->smb_data, "wwlqq",
	    &StructSize,		 
	    &reserved1,			 
	    &reserved2,			 
	    &smb2fid.persistent,	 
	    &smb2fid.temporal);		 
	if (rc)
		return (SDRC_ERROR);
	if (StructSize != 24)
		return (SDRC_ERROR);
	status = smb2sr_lookup_fid(sr, &smb2fid);
	if (status) {
		smb2sr_put_error(sr, status);
		return (SDRC_SUCCESS);
	}
	of = sr->fid_ofile;
	if ((of->f_node->flags & NODE_FLAGS_WRITE_THROUGH) == 0)
		(void) smb_fsop_commit(sr, of->f_cr, of->f_node);
	(void) smb_mbc_encodef(
	    &sr->reply, "wwl",
	    4,	 	 
	    0);  		 
	return (SDRC_SUCCESS);
}