static int php_mb_parse_encoding_list(const char *value, int value_length,
                                      mbfl_encoding ***return_list,
                                      int *return_size, int persistent) {
  int n, l, size, bauto, ret = 1;
  char *p, *p1, *p2, *endp, *tmpstr;
  mbfl_encoding *encoding;
  mbfl_no_encoding *src;
  mbfl_encoding **entry, **list;
  list = nullptr;
  if (value == nullptr || value_length <= 0) {
    if (return_list) {
      *return_list = nullptr;
    }
    if (return_size) {
      *return_size = 0;
    }
    return 0;
  } else {
    mbfl_no_encoding *identify_list;
    int identify_list_size;
    identify_list = MBSTRG(default_detect_order_list);
    identify_list_size = MBSTRG(default_detect_order_list_size);
    if (value[0]=='"' && value[value_length-1]=='"' && value_length>2) {
      tmpstr = (char *)strndup(value+1, value_length-2);
      value_length -= 2;
    }
    else
      tmpstr = (char *)strndup(value, value_length);
    if (tmpstr == nullptr) {
      return 0;
    }
    endp = tmpstr + value_length;
    n = 1;
    p1 = tmpstr;
    while ((p2 = (char*)string_memnstr(p1, ",", 1, endp)) != nullptr) {
      p1 = p2 + 1;
      n++;
    }
    size = n + identify_list_size;
    list = (mbfl_encoding **)calloc(size, sizeof(mbfl_encoding*));
    if (list != nullptr) {
      entry = list;
      n = 0;
      bauto = 0;
      p1 = tmpstr;
      do {
        p2 = p = (char*)string_memnstr(p1, ",", 1, endp);
        if (p == nullptr) {
          p = endp;
        }
        *p = '\0';
        while (p1 < p && (*p1 == ' ' || *p1 == '\t')) {
          p1++;
        }
        p--;
        while (p > p1 && (*p == ' ' || *p == '\t')) {
          *p = '\0';
          p--;
        }
        if (strcasecmp(p1, "auto") == 0) {
          if (!bauto) {
            bauto = 1;
            l = identify_list_size;
            src = identify_list;
            for (int i = 0; i < l; i++) {
              *entry++ = (mbfl_encoding*) mbfl_no2encoding(*src++);
              n++;
            }
          }
        } else {
          encoding = (mbfl_encoding*) mbfl_name2encoding(p1);
          if (encoding != nullptr) {
            *entry++ = encoding;
            n++;
          } else {
            ret = 0;
          }
        }
        p1 = p2 + 1;
      } while (n < size && p2 != nullptr);
      if (n > 0) {
        if (return_list) {
          *return_list = list;
        } else {
          free(list);
        }
      } else {
        free(list);
        if (return_list) {
          *return_list = nullptr;
        }
        ret = 0;
      }
      if (return_size) {
        *return_size = n;
      }
    } else {
      if (return_list) {
        *return_list = nullptr;
      }
      if (return_size) {
        *return_size = 0;
      }
      ret = 0;
    }
    free(tmpstr);
  }
  return ret;
}