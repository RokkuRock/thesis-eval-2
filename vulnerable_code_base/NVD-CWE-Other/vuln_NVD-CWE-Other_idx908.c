__u32 secure_tcp_sequence_number(__be32 saddr, __be32 daddr,
				 __be16 sport, __be16 dport)
{
	__u32 seq;
	__u32 hash[4];
	struct keydata *keyptr = get_keyptr();
	hash[0] = (__force u32)saddr;
	hash[1] = (__force u32)daddr;
	hash[2] = ((__force u16)sport << 16) + (__force u16)dport;
	hash[3] = keyptr->secret[11];
	seq = half_md4_transform(hash, keyptr->secret) & HASH_MASK;
	seq += keyptr->count;
	seq += ktime_to_ns(ktime_get_real()) >> 6;
	return seq;
}