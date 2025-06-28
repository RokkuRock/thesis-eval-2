bool HHVM_FUNCTION(mb_send_mail,
                   const String& to,
                   const String& subject,
                   const String& message,
                   const Variant& opt_headers,
                   const Variant& opt_extra_cmd) {
  const String headers = convertArg(opt_headers);
  const String extra_cmd = convertArg(opt_extra_cmd);
  mbfl_memory_device device;
  mbfl_memory_device_init(&device, 0, 0);
  mbfl_string orig_str, conv_str;
  mbfl_string_init(&orig_str);
  mbfl_string_init(&conv_str);
  mbfl_no_encoding
    tran_cs,   
    head_enc,   
    body_enc;   
  tran_cs = mbfl_no_encoding_utf8;
  head_enc = mbfl_no_encoding_base64;
  body_enc = mbfl_no_encoding_base64;
  const mbfl_language *lang = mbfl_no2language(MBSTRG(current_language));
  if (lang != nullptr) {
    tran_cs = lang->mail_charset;
    head_enc = lang->mail_header_encoding;
    body_enc = lang->mail_body_encoding;
  }
  Array ht_headers;
  if (!headers.empty()) {
    _php_mbstr_parse_mail_headers(ht_headers, headers.data(), headers.size());
  }
  struct {
    unsigned int cnt_type:1;
    unsigned int cnt_trans_enc:1;
  } suppressed_hdrs = { 0, 0 };
  static const StaticString s_CONTENT_TYPE("CONTENT-TYPE");
  String s = ht_headers[s_CONTENT_TYPE].toString();
  if (!s.isNull()) {
    char *tmp;
    char *param_name;
    char *charset = nullptr;
    char *p = const_cast<char*>(strchr(s.data(), ';'));
    if (p != nullptr) {
      do {
        ++p;
      } while (*p == ' ' || *p == '\t');
      if (*p != '\0') {
        if ((param_name = strtok_r(p, "= ", &tmp)) != nullptr) {
          if (strcasecmp(param_name, "charset") == 0) {
            mbfl_no_encoding _tran_cs = tran_cs;
            charset = strtok_r(nullptr, "= ", &tmp);
            if (charset != nullptr) {
              _tran_cs = mbfl_name2no_encoding(charset);
            }
            if (_tran_cs == mbfl_no_encoding_invalid) {
              raise_warning("Unsupported charset \"%s\" - "
                              "will be regarded as ascii", charset);
              _tran_cs = mbfl_no_encoding_ascii;
            }
            tran_cs = _tran_cs;
          }
        }
      }
    }
    suppressed_hdrs.cnt_type = 1;
  }
  static const StaticString
         s_CONTENT_TRANSFER_ENCODING("CONTENT-TRANSFER-ENCODING");
  s = ht_headers[s_CONTENT_TRANSFER_ENCODING].toString();
  if (!s.isNull()) {
    mbfl_no_encoding _body_enc = mbfl_name2no_encoding(s.data());
    switch (_body_enc) {
    case mbfl_no_encoding_base64:
    case mbfl_no_encoding_7bit:
    case mbfl_no_encoding_8bit:
      body_enc = _body_enc;
      break;
    default:
      raise_warning("Unsupported transfer encoding \"%s\" - "
                      "will be regarded as 8bit", s.data());
      body_enc =  mbfl_no_encoding_8bit;
      break;
    }
    suppressed_hdrs.cnt_trans_enc = 1;
  }
  char *to_r = nullptr;
  int err = 0;
  if (!to.empty()) {
    int to_len = to.size();
    if (to_len > 0) {
      to_r = strndup(to.data(), to_len);
      for (; to_len; to_len--) {
        if (!isspace((unsigned char)to_r[to_len - 1])) {
          break;
        }
        to_r[to_len - 1] = '\0';
      }
      for (int i = 0; to_r[i]; i++) {
        if (iscntrl((unsigned char)to_r[i])) {
          SKIP_LONG_HEADER_SEP_MBSTRING(to_r, i);
          to_r[i] = ' ';
        }
      }
    } else {
      to_r = (char*)to.data();
    }
  } else {
    raise_warning("Missing To: field");
    err = 1;
  }
  String encoded_subject;
  if (!subject.isNull()) {
    orig_str.no_language = MBSTRG(current_language);
    orig_str.val = (unsigned char *)subject.data();
    orig_str.len = subject.size();
    orig_str.no_encoding = MBSTRG(current_internal_encoding)->no_encoding;
    if (orig_str.no_encoding == mbfl_no_encoding_invalid
        || orig_str.no_encoding == mbfl_no_encoding_pass) {
      mbfl_encoding *encoding =
        (mbfl_encoding*) mbfl_identify_encoding2(&orig_str,
           (const mbfl_encoding**) MBSTRG(current_detect_order_list),
           MBSTRG(current_detect_order_list_size), MBSTRG(strict_detection));
      orig_str.no_encoding = encoding != nullptr
                           ? encoding->no_encoding
                           : mbfl_no_encoding_invalid;
    }
    mbfl_string *pstr = mbfl_mime_header_encode
      (&orig_str, &conv_str, tran_cs, head_enc,
       "\n", sizeof("Subject: [PHP-jp nnnnnnnn]"));
    if (pstr != nullptr) {
      encoded_subject = String(reinterpret_cast<char*>(pstr->val),
                               pstr->len,
                               AttachString);
    }
  } else {
    raise_warning("Missing Subject: field");
    err = 1;
  }
  String encoded_message;
  if (!message.empty()) {
    orig_str.no_language = MBSTRG(current_language);
    orig_str.val = (unsigned char*)message.data();
    orig_str.len = message.size();
    orig_str.no_encoding = MBSTRG(current_internal_encoding)->no_encoding;
    if (orig_str.no_encoding == mbfl_no_encoding_invalid
        || orig_str.no_encoding == mbfl_no_encoding_pass) {
      mbfl_encoding *encoding =
        (mbfl_encoding*) mbfl_identify_encoding2(&orig_str,
           (const mbfl_encoding**) MBSTRG(current_detect_order_list),
           MBSTRG(current_detect_order_list_size), MBSTRG(strict_detection));
      orig_str.no_encoding = encoding != nullptr
                           ? encoding->no_encoding
                           : mbfl_no_encoding_invalid;
    }
    mbfl_string *pstr = nullptr;
    {
      mbfl_string tmpstr;
      if (mbfl_convert_encoding(&orig_str, &tmpstr, tran_cs) != nullptr) {
        tmpstr.no_encoding = mbfl_no_encoding_8bit;
        pstr = mbfl_convert_encoding(&tmpstr, &conv_str, body_enc);
        free(tmpstr.val);
      }
    }
    if (pstr != nullptr) {
      encoded_message = String(reinterpret_cast<char*>(pstr->val),
                               pstr->len,
                               AttachString);
    }
  } else {
    raise_warning("Empty message body");
  }
#define PHP_MBSTR_MAIL_MIME_HEADER1 "Mime-Version: 1.0"
#define PHP_MBSTR_MAIL_MIME_HEADER2 "Content-Type: text/plain"
#define PHP_MBSTR_MAIL_MIME_HEADER3 "; charset="
#define PHP_MBSTR_MAIL_MIME_HEADER4 "Content-Transfer-Encoding: "
  if (!headers.empty()) {
    const char *p = headers.data();
    int n = headers.size();
    mbfl_memory_device_strncat(&device, p, n);
    if (n > 0 && p[n - 1] != '\n') {
      mbfl_memory_device_strncat(&device, "\n", 1);
    }
  }
  mbfl_memory_device_strncat(&device, PHP_MBSTR_MAIL_MIME_HEADER1,
                             sizeof(PHP_MBSTR_MAIL_MIME_HEADER1) - 1);
  mbfl_memory_device_strncat(&device, "\n", 1);
  if (!suppressed_hdrs.cnt_type) {
    mbfl_memory_device_strncat(&device, PHP_MBSTR_MAIL_MIME_HEADER2,
                               sizeof(PHP_MBSTR_MAIL_MIME_HEADER2) - 1);
    char *p = (char *)mbfl_no2preferred_mime_name(tran_cs);
    if (p != nullptr) {
      mbfl_memory_device_strncat(&device, PHP_MBSTR_MAIL_MIME_HEADER3,
                                 sizeof(PHP_MBSTR_MAIL_MIME_HEADER3) - 1);
      mbfl_memory_device_strcat(&device, p);
    }
    mbfl_memory_device_strncat(&device, "\n", 1);
  }
  if (!suppressed_hdrs.cnt_trans_enc) {
    mbfl_memory_device_strncat(&device, PHP_MBSTR_MAIL_MIME_HEADER4,
                               sizeof(PHP_MBSTR_MAIL_MIME_HEADER4) - 1);
    const char *p = (char *)mbfl_no2preferred_mime_name(body_enc);
    if (p == nullptr) {
      p = "7bit";
    }
    mbfl_memory_device_strcat(&device, p);
    mbfl_memory_device_strncat(&device, "\n", 1);
  }
  mbfl_memory_device_unput(&device);
  mbfl_memory_device_output('\0', &device);
  char *all_headers = (char *)device.buffer;
  String cmd = string_escape_shell_cmd(extra_cmd.c_str());
  bool ret = (!err && php_mail(to_r, encoded_subject.data(),
                               encoded_message.data(),
                               all_headers, cmd.data()));
  mbfl_memory_device_clear(&device);
  return ret;
}