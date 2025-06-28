_hivex_get_children (hive_h *h, hive_node_h node,
                     hive_node_h **children_ret, size_t **blocks_ret,
                     int flags)
{
  if (!IS_VALID_BLOCK (h, node) || !block_id_eq (h, node, "nk")) {
    SET_ERRNO (EINVAL, "invalid block or not an 'nk' block");
    return -1;
  }
  struct ntreg_nk_record *nk =
    (struct ntreg_nk_record *) ((char *) h->addr + node);
  size_t nr_subkeys_in_nk = le32toh (nk->nr_subkeys);
  offset_list children, blocks;
  _hivex_init_offset_list (h, &children);
  _hivex_init_offset_list (h, &blocks);
  if (nr_subkeys_in_nk == 0)
    goto out;
  if (nr_subkeys_in_nk > HIVEX_MAX_SUBKEYS) {
    SET_ERRNO (ERANGE,
               "nr_subkeys_in_nk > HIVEX_MAX_SUBKEYS (%zu > %d)",
               nr_subkeys_in_nk, HIVEX_MAX_SUBKEYS);
    goto error;
  }
  _hivex_set_offset_list_limit (&children, nr_subkeys_in_nk);
  _hivex_set_offset_list_limit (&blocks, HIVEX_MAX_SUBKEYS);
  if (_hivex_grow_offset_list (&children, nr_subkeys_in_nk) == -1)
    goto error;
  size_t subkey_lf = le32toh (nk->subkey_lf);
  subkey_lf += 0x1000;
  if (!IS_VALID_BLOCK (h, subkey_lf)) {
    SET_ERRNO (EFAULT,
               "subkey_lf is not a valid block (0x%zx)", subkey_lf);
    goto error;
  }
  if (_get_children (h, subkey_lf, &children, &blocks, flags) == -1)
    goto error;
  size_t nr_children = _hivex_get_offset_list_length (&children);
  if (nr_subkeys_in_nk != nr_children) {
    if (!h->unsafe) {
      SET_ERRNO (ENOTSUP,
                 "nr_subkeys_in_nk = %zu "
                 "is not equal to number of children read %zu",
                 nr_subkeys_in_nk, nr_children);
      goto error;
    } else {
      DEBUG (2,
             "nr_subkeys_in_nk = %zu "
             "is not equal to number of children read %zu",
             nr_subkeys_in_nk, nr_children);
    }
  }
 out:
#if 0
  if (h->msglvl >= 2) {
    fprintf (stderr, "%s: %s: children = ", "hivex", __func__);
    _hivex_print_offset_list (&children, stderr);
    fprintf (stderr, "\n%s: %s: blocks = ", "hivex", __func__);
    _hivex_print_offset_list (&blocks, stderr);
    fprintf (stderr, "\n");
  }
#endif
  *children_ret = _hivex_return_offset_list (&children);
  *blocks_ret = _hivex_return_offset_list (&blocks);
  if (!*children_ret || !*blocks_ret)
    goto error;
  return 0;
 error:
  _hivex_free_offset_list (&children);
  _hivex_free_offset_list (&blocks);
  return -1;
}