BOOL rdp_decrypt(rdpRdp* rdp, STREAM* s, int length, UINT16 securityFlags)
{
	BYTE cmac[8];
	BYTE wmac[8];
	if (rdp->settings->EncryptionMethods == ENCRYPTION_METHOD_FIPS)
	{
		UINT16 len;
		BYTE version, pad;
		BYTE* sig;
		if (stream_get_left(s) < 12)
			return FALSE;
		stream_read_UINT16(s, len);  
		stream_read_BYTE(s, version);  
		stream_read_BYTE(s, pad);
		sig = s->p;
		stream_seek(s, 8);	 
		length -= 12;
		if (!security_fips_decrypt(s->p, length, rdp))
		{
			printf("FATAL: cannot decrypt\n");
			return FALSE;  
		}
		if (!security_fips_check_signature(s->p, length - pad, sig, rdp))
		{
			printf("FATAL: invalid packet signature\n");
			return FALSE;  
		}
		s->size -= pad;
		return TRUE;
	}
	if (stream_get_left(s) < 8)
		return FALSE;
	stream_read(s, wmac, sizeof(wmac));
	length -= sizeof(wmac);
	security_decrypt(s->p, length, rdp);
	if (securityFlags & SEC_SECURE_CHECKSUM)
		security_salted_mac_signature(rdp, s->p, length, FALSE, cmac);
	else
		security_mac_signature(rdp, s->p, length, cmac);
	if (memcmp(wmac, cmac, sizeof(wmac)) != 0)
	{
		printf("WARNING: invalid packet signature\n");
	}
	return TRUE;
}