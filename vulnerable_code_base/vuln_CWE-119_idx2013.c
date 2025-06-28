void mt_init(mtrand *mt, uint32_t seed) {
  int i;
  mt->mt_buffer_[0] = seed;
  mt->mt_index_ = MT_LEN;
  for (i = 1; i < MT_LEN; i++) {
    mt->mt_buffer_[i] =
	(1812433253UL * (mt->mt_buffer_[i-1] ^ 
			 (mt->mt_buffer_[i-1] >> 30)) + i);
  }
}