static inline bool key_is_instantiated(const struct key *key)
{
	return test_bit(KEY_FLAG_INSTANTIATED, &key->flags) &&
		!test_bit(KEY_FLAG_NEGATIVE, &key->flags);
}