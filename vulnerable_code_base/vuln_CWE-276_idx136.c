resolve_op_end (FlatpakTransaction *self,
                FlatpakTransactionOperation *op,
                const char *checksum,
                GFile *sideload_path,
                GBytes *metadata_bytes)
{
  g_autoptr(GBytes) old_metadata_bytes = NULL;
  old_metadata_bytes = load_deployed_metadata (self, op->ref, NULL, NULL);
  mark_op_resolved (op, checksum, sideload_path, metadata_bytes, old_metadata_bytes);
  emit_eol_and_maybe_skip (self, op);
 }