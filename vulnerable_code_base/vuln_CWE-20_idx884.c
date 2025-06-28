static struct dst_entry *ip6_sk_dst_check(struct sock *sk,
					  struct dst_entry *dst,
					  const struct flowi6 *fl6)
{
	struct ipv6_pinfo *np = inet6_sk(sk);
	struct rt6_info *rt = (struct rt6_info *)dst;
	if (!dst)
		goto out;
	if (ip6_rt_check(&rt->rt6i_dst, &fl6->daddr, np->daddr_cache) ||
#ifdef CONFIG_IPV6_SUBTREES
	    ip6_rt_check(&rt->rt6i_src, &fl6->saddr, np->saddr_cache) ||
#endif
	    (fl6->flowi6_oif && fl6->flowi6_oif != dst->dev->ifindex)) {
		dst_release(dst);
		dst = NULL;
	}
out:
	return dst;
}