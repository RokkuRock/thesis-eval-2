int ksmbd_decode_ntlmssp_auth_blob(struct authenticate_message *authblob,
				   int blob_len, struct ksmbd_conn *conn,
				   struct ksmbd_session *sess)
{
	char *domain_name;
	unsigned int nt_off, dn_off;
	unsigned short nt_len, dn_len;
#ifdef CONFIG_SMB_INSECURE_SERVER
	unsigned int lm_off;
	unsigned short lm_len;
#endif
	int ret;
	if (blob_len < sizeof(struct authenticate_message)) {
		ksmbd_debug(AUTH, "negotiate blob len %d too small\n",
			    blob_len);
		return -EINVAL;
	}
	if (memcmp(authblob->Signature, "NTLMSSP", 8)) {
		ksmbd_debug(AUTH, "blob signature incorrect %s\n",
			    authblob->Signature);
		return -EINVAL;
	}
	nt_off = le32_to_cpu(authblob->NtChallengeResponse.BufferOffset);
	nt_len = le16_to_cpu(authblob->NtChallengeResponse.Length);
	dn_off = le32_to_cpu(authblob->DomainName.BufferOffset);
	dn_len = le16_to_cpu(authblob->DomainName.Length);
	if (blob_len < (u64)dn_off + dn_len || blob_len < (u64)nt_off + nt_len)
		return -EINVAL;
#ifdef CONFIG_SMB_INSECURE_SERVER
	lm_off = le32_to_cpu(authblob->LmChallengeResponse.BufferOffset);
	lm_len = le16_to_cpu(authblob->LmChallengeResponse.Length);
	if (blob_len < (u64)lm_off + lm_len)
		return -EINVAL;
	if (nt_len == CIFS_AUTH_RESP_SIZE) {
		if (le32_to_cpu(authblob->NegotiateFlags) &
		    NTLMSSP_NEGOTIATE_EXTENDED_SEC)
			return __ksmbd_auth_ntlmv2(sess,
						   (char *)authblob + lm_off,
						   (char *)authblob + nt_off,
						   conn->ntlmssp.cryptkey);
		else
			return ksmbd_auth_ntlm(sess, (char *)authblob +
				nt_off, conn->ntlmssp.cryptkey);
	}
#endif
	domain_name = smb_strndup_from_utf16((const char *)authblob + dn_off,
					     dn_len, true, conn->local_nls);
	if (IS_ERR(domain_name))
		return PTR_ERR(domain_name);
	ksmbd_debug(AUTH, "decode_ntlmssp_authenticate_blob dname%s\n",
		    domain_name);
	ret = ksmbd_auth_ntlmv2(conn, sess,
				(struct ntlmv2_resp *)((char *)authblob + nt_off),
				nt_len - CIFS_ENCPWD_SIZE,
				domain_name, conn->ntlmssp.cryptkey);
	kfree(domain_name);
	if (conn->ntlmssp.client_flags & NTLMSSP_NEGOTIATE_KEY_XCH) {
		struct arc4_ctx *ctx_arc4;
		unsigned int sess_key_off, sess_key_len;
		sess_key_off = le32_to_cpu(authblob->SessionKey.BufferOffset);
		sess_key_len = le16_to_cpu(authblob->SessionKey.Length);
		if (blob_len < (u64)sess_key_off + sess_key_len)
			return -EINVAL;
		ctx_arc4 = kmalloc(sizeof(*ctx_arc4), GFP_KERNEL);
		if (!ctx_arc4)
			return -ENOMEM;
		cifs_arc4_setkey(ctx_arc4, sess->sess_key,
				 SMB2_NTLMV2_SESSKEY_SIZE);
		cifs_arc4_crypt(ctx_arc4, sess->sess_key,
				(char *)authblob + sess_key_off, sess_key_len);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)
		kfree_sensitive(ctx_arc4);
#else
		memzero_explicit((void *)ctx_arc4, sizeof(*ctx_arc4));
		kfree(ctx_arc4);
#endif
	}
	return ret;
}