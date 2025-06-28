char *string_crypt(const char *key, const char *salt) {
  assert(key);
  assert(salt);
  char random_salt[12];
  if (!*salt) {
    memcpy(random_salt,"$1$",3);
    ito64(random_salt+3,rand(),8);
    random_salt[11] = '\0';
    return string_crypt(key, random_salt);
  }
  if ((strlen(salt) > sizeof("$2X$00$")) &&
    (salt[0] == '$') &&
    (salt[1] == '2') &&
    (salt[2] >= 'a') && (salt[2] <= 'z') &&
    (salt[3] == '$') &&
    (salt[4] >= '0') && (salt[4] <= '3') &&
    (salt[5] >= '0') && (salt[5] <= '9') &&
    (salt[6] == '$')) {
    char output[61];
    if (php_crypt_blowfish_rn(key, salt, output, sizeof(output))) {
      return strdup(output);
    }
  } else {
#ifdef USE_PHP_CRYPT_R
    return php_crypt_r(key, salt);
#else
    static Mutex mutex;
    Lock lock(mutex);
    char *crypt_res = crypt(key,salt);
    if (crypt_res) {
      return strdup(crypt_res);
    }
#endif
  }
  return ((salt[0] == '*') && (salt[1] == '0'))
                  ? strdup("*1") : strdup("*0");
}