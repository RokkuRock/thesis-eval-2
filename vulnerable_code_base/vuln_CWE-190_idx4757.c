static void lsr_read_extend_class(GF_LASeRCodec *lsr, char **out_data, u32 *out_len, const char *name)
{
	u32 len;
	GF_LSR_READ_INT(lsr, len, lsr->info->cfg.extensionIDBits, "reserved");
	len = lsr_read_vluimsbf5(lsr, "len");
	gf_bs_read_long_int(lsr->bs, len);
	if (out_data) *out_data = NULL;
	if (out_len) *out_len = 0;
}