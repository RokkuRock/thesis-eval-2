static u32 __ipv6_select_ident(struct net *net, u32 hashrnd,
			       const struct in6_addr *dst,
			       const struct in6_addr *src)
{
	u32 hash, id;
	hash = __ipv6_addr_jhash(dst, hashrnd);
	hash = __ipv6_addr_jhash(src, hash);
	hash ^= net_hash_mix(net);
	id = ip_idents_reserve(hash, 1);
	if (unlikely(!id))
		id = 1 << 31;
	return id;
}