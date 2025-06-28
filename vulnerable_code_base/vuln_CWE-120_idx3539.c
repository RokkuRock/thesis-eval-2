PJ_DEF(pj_status_t) pjmedia_rtp_decode_rtp2(
					    pjmedia_rtp_session *ses,
					    const void *pkt, int pkt_len,
					    const pjmedia_rtp_hdr **hdr,
					    pjmedia_rtp_dec_hdr *dec_hdr,
					    const void **payload,
					    unsigned *payloadlen)
{
    int offset;
    PJ_UNUSED_ARG(ses);
    *hdr = (pjmedia_rtp_hdr*)pkt;
    if ((*hdr)->v != RTP_VERSION) {
	return PJMEDIA_RTP_EINVER;
    }
    offset = sizeof(pjmedia_rtp_hdr) + ((*hdr)->cc * sizeof(pj_uint32_t));
    if ((*hdr)->x) {
        if (offset + sizeof (pjmedia_rtp_ext_hdr) > (unsigned)pkt_len)
            return PJMEDIA_RTP_EINLEN;
        dec_hdr->ext_hdr = (pjmedia_rtp_ext_hdr*)(((pj_uint8_t*)pkt) + offset);
        dec_hdr->ext = (pj_uint32_t*)(dec_hdr->ext_hdr + 1);
        dec_hdr->ext_len = pj_ntohs((dec_hdr->ext_hdr)->length);
        offset += ((dec_hdr->ext_len + 1) * sizeof(pj_uint32_t));
    } else {
	dec_hdr->ext_hdr = NULL;
	dec_hdr->ext = NULL;
	dec_hdr->ext_len = 0;
    }
    if (offset > pkt_len)
	return PJMEDIA_RTP_EINLEN;
    *payload = ((pj_uint8_t*)pkt) + offset;
    *payloadlen = pkt_len - offset;
    if ((*hdr)->p && *payloadlen > 0) {
	pj_uint8_t pad_len;
	pad_len = ((pj_uint8_t*)(*payload))[*payloadlen - 1];
	if (pad_len <= *payloadlen)
	    *payloadlen -= pad_len;
    }
    return PJ_SUCCESS;
}