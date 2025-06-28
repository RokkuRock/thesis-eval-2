bool HHVM_FUNCTION(mb_parse_str,
                   const String& encoded_string,
                   VRefParam result  ) {
  php_mb_encoding_handler_info_t info;
  info.data_type              = PARSE_STRING;
  info.separator              = ";&";
  info.force_register_globals = false;
  info.report_errors          = 1;
  info.to_encoding            = MBSTRG(current_internal_encoding);
  info.to_language            = MBSTRG(current_language);
  info.from_encodings         = MBSTRG(http_input_list);
  info.num_from_encodings     = MBSTRG(http_input_list_size);
  info.from_language          = MBSTRG(current_language);
  char *encstr = strndup(encoded_string.data(), encoded_string.size());
  Array resultArr = Array::Create();
  mbfl_encoding *detected =
    _php_mb_encoding_handler_ex(&info, resultArr, encstr);
  free(encstr);
  result.assignIfRef(resultArr);
  MBSTRG(http_input_identify) = detected;
  return detected != nullptr;
}