uint32_t mt_random (mtrand *mt) {
  uint32_t y;
  unsigned long mag01[2];
  mag01[0] = 0;
  mag01[1] = MATRIX_A;
  if (mt->mt_index_ >= MT_LEN) {
    int kk;
    for (kk = 0; kk < MT_LEN - MT_IA; kk++) {
      y = (mt->mt_buffer_[kk] & UPPER_MASK) | 
	(mt->mt_buffer_[kk + 1] & LOWER_MASK);
      mt->mt_buffer_[kk] = mt->mt_buffer_[kk + MT_IA] ^ 
	(y >> 1) ^ mag01[y & 0x1UL];
    }
    for (;kk < MT_LEN - 1; kk++) {
      y = (mt->mt_buffer_[kk] & UPPER_MASK) | 
	(mt->mt_buffer_[kk + 1] & LOWER_MASK);
      mt->mt_buffer_[kk] = mt->mt_buffer_[kk + (MT_IA - MT_LEN)] ^ 
	(y >> 1) ^ mag01[y & 0x1UL];
    }
    y = (mt->mt_buffer_[MT_LEN - 1] & UPPER_MASK) | 
      (mt->mt_buffer_[0] & LOWER_MASK);
    mt->mt_buffer_[MT_LEN - 1] = mt->mt_buffer_[MT_IA - 1] ^ 
      (y >> 1) ^ mag01[y & 0x1UL];
    mt->mt_index_ = 0;
  }
  y = mt->mt_buffer_[mt->mt_index_++];
  y ^= (y >> 11);
  y ^= (y << 7) & 0x9d2c5680UL;
  y ^= (y << 15) & 0xefc60000UL;
  y ^= (y >> 18);
  return y;
}