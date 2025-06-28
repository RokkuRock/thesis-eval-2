static char *BF_crypt(const char *key, const char *setting,
  char *output, int size,
  BF_word min)
{
#if BF_ASM
  extern void _BF_body_r(BF_ctx *ctx);
#endif
  static const unsigned char flags_by_subtype[26] =
    {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 4, 0};
  struct {
    BF_ctx ctx;
    BF_key expanded_key;
    union {
      BF_word salt[4];
      BF_word output[6];
    } binary;
  } data;
  BF_word L, R;
  BF_word tmp1, tmp2, tmp3, tmp4;
  BF_word *ptr;
  BF_word count;
  int i;
  if (size < 7 + 22 + 31 + 1) {
    __set_errno(ERANGE);
    return NULL;
  }
  if (setting[0] != '$' ||
      setting[1] != '2' ||
      setting[2] < 'a' || setting[2] > 'z' ||
      !flags_by_subtype[(unsigned int)(unsigned char)setting[2] - 'a'] ||
      setting[3] != '$' ||
      setting[4] < '0' || setting[4] > '3' ||
      setting[5] < '0' || setting[5] > '9' ||
      (setting[4] == '3' && setting[5] > '1') ||
      setting[6] != '$') {
    __set_errno(EINVAL);
    return NULL;
  }
  count = (BF_word)1 << ((setting[4] - '0') * 10 + (setting[5] - '0'));
  if (count < min || BF_decode(data.binary.salt, &setting[7], 16)) {
    __set_errno(EINVAL);
    return NULL;
  }
  BF_swap(data.binary.salt, 4);
  BF_set_key(key, data.expanded_key, data.ctx.P,
      flags_by_subtype[(unsigned int)(unsigned char)setting[2] - 'a']);
  memcpy(data.ctx.S, BF_init_state.S, sizeof(data.ctx.S));
  L = R = 0;
  for (i = 0; i < BF_N + 2; i += 2) {
    L ^= data.binary.salt[i & 2];
    R ^= data.binary.salt[(i & 2) + 1];
    BF_ENCRYPT;
    data.ctx.P[i] = L;
    data.ctx.P[i + 1] = R;
  }
  ptr = data.ctx.S[0];
  do {
    ptr += 4;
    L ^= data.binary.salt[(BF_N + 2) & 3];
    R ^= data.binary.salt[(BF_N + 3) & 3];
    BF_ENCRYPT;
    *(ptr - 4) = L;
    *(ptr - 3) = R;
    L ^= data.binary.salt[(BF_N + 4) & 3];
    R ^= data.binary.salt[(BF_N + 5) & 3];
    BF_ENCRYPT;
    *(ptr - 2) = L;
    *(ptr - 1) = R;
  } while (ptr < &data.ctx.S[3][0xFF]);
  do {
    int done;
    for (i = 0; i < BF_N + 2; i += 2) {
      data.ctx.P[i] ^= data.expanded_key[i];
      data.ctx.P[i + 1] ^= data.expanded_key[i + 1];
    }
    done = 0;
    do {
      BF_body();
      if (done)
        break;
      done = 1;
      tmp1 = data.binary.salt[0];
      tmp2 = data.binary.salt[1];
      tmp3 = data.binary.salt[2];
      tmp4 = data.binary.salt[3];
      for (i = 0; i < BF_N; i += 4) {
        data.ctx.P[i] ^= tmp1;
        data.ctx.P[i + 1] ^= tmp2;
        data.ctx.P[i + 2] ^= tmp3;
        data.ctx.P[i + 3] ^= tmp4;
      }
      data.ctx.P[16] ^= tmp1;
      data.ctx.P[17] ^= tmp2;
    } while (1);
  } while (--count);
  for (i = 0; i < 6; i += 2) {
    L = BF_magic_w[i];
    R = BF_magic_w[i + 1];
    count = 64;
    do {
      BF_ENCRYPT;
    } while (--count);
    data.binary.output[i] = L;
    data.binary.output[i + 1] = R;
  }
  memcpy(output, setting, 7 + 22 - 1);
  output[7 + 22 - 1] = BF_itoa64[(int)
    BF_atoi64[(int)setting[7 + 22 - 1] - 0x20] & 0x30];
  BF_swap(data.binary.output, 6);
  BF_encode(&output[7 + 22], data.binary.output, 23);
  output[7 + 22 + 31] = '\0';
  return output;
}