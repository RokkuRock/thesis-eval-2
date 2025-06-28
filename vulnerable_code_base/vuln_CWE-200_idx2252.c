entry_guard_obeys_restriction(const entry_guard_t *guard,
                              const entry_guard_restriction_t *rst)
{
  tor_assert(guard);
  if (! rst)
    return 1;  
  return tor_memneq(guard->identity, rst->exclude_id, DIGEST_LEN);
}