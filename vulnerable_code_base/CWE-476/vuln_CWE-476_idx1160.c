u32 gf_bs_read_ue_log_idx3(GF_BitStream *bs, const char *fname, s32 idx1, s32 idx2, s32 idx3)
{
	u32 val=0, code;
	s32 nb_lead = -1;
	u32 bits = 0;
	for (code=0; !code; nb_lead++) {
		if (nb_lead>=32) {
			if (!gf_bs_available(bs)) {
				GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[Core] exp-golomb read failed, not enough bits in bitstream !\n"));
			} else {
				GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[Core] corrupted exp-golomb code, %d leading zeros, max 31 allowed !\n", nb_lead));
			}
			return 0;
		}
		code = gf_bs_read_int(bs, 1);
		bits++;
	}
	if (nb_lead) {
		val = gf_bs_read_int(bs, nb_lead);
		val += (1 << nb_lead) - 1;
		bits += nb_lead;
	}
	if (fname) {
		gf_bs_log_idx(bs, bits, fname, val, idx1, idx2, idx3);
	}
	return val;
}