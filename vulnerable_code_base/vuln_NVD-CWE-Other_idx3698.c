static inline __u32 secure_dccpv6_sequence_number(__be32 *saddr, __be32 *daddr,
						  __be16 sport, __be16 dport   )
{
	return secure_tcpv6_sequence_number(saddr, daddr, sport, dport);
}