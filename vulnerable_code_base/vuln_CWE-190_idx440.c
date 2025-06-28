static Fixed lsr_translate_scale(GF_LASeRCodec *lsr, u32 val)
{
	if (val >> (lsr->coord_bits-1) ) {
		s32 v = val - (1<<lsr->coord_bits);
		return INT2FIX(v) / 256 ;
	} else {
		return INT2FIX(val) / 256;
	}
}