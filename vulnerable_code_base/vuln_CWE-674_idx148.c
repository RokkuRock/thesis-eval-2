static int nla_validate_array(const struct nlattr *head, int len, int maxtype,
			      const struct nla_policy *policy,
			      struct netlink_ext_ack *extack,
			      unsigned int validate)
{
	const struct nlattr *entry;
	int rem;
	nla_for_each_attr(entry, head, len, rem) {
		int ret;
		if (nla_len(entry) == 0)
			continue;
		if (nla_len(entry) < NLA_HDRLEN) {
			NL_SET_ERR_MSG_ATTR(extack, entry,
					    "Array element too short");
			return -ERANGE;
		}
		ret = __nla_validate(nla_data(entry), nla_len(entry),
				     maxtype, policy, validate, extack);
		if (ret < 0)
			return ret;
	}
	return 0;
}