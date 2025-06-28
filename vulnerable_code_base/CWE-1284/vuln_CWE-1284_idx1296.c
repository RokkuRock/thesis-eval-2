static void validate_dex_header(const dex_header* dh,
                                size_t dexsize,
                                int support_dex_version) {
  bool supported = false;
  switch (support_dex_version) {
  case 38:
    supported = supported ||
                !memcmp(dh->magic, DEX_HEADER_DEXMAGIC_V38, sizeof(dh->magic));
    FALLTHROUGH_INTENDED;  
  case 37:
    supported = supported ||
                !memcmp(dh->magic, DEX_HEADER_DEXMAGIC_V37, sizeof(dh->magic));
    FALLTHROUGH_INTENDED;  
  case 35:
    supported = supported ||
                !memcmp(dh->magic, DEX_HEADER_DEXMAGIC_V35, sizeof(dh->magic));
    break;
  default:
    not_reached_log("Unrecognized support_dex_version %d\n",
                    support_dex_version);
  }
  always_assert_log(supported, "Bad dex magic %s for support_dex_version %d\n",
                    dh->magic, support_dex_version);
  always_assert_log(
      dh->file_size == dexsize,
      "Reported size in header (%zu) does not match file size (%u)\n",
      dexsize,
      dh->file_size);
  auto off = (uint64_t)dh->class_defs_off;
  auto limit = off + dh->class_defs_size * sizeof(dex_class_def);
  always_assert_log(off < dexsize, "class_defs_off out of range");
  always_assert_log(limit <= dexsize, "invalid class_defs_size");
}