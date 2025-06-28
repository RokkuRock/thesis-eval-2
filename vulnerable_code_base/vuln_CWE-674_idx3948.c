static int __nla_validate_parse(const struct nlattr *head, int len, int maxtype,
				const struct nla_policy *policy,
				unsigned int validate,
				struct netlink_ext_ack *extack,
				struct nlattr **tb)
{
	const struct nlattr *nla;
	int rem;
	if (tb)
		memset(tb, 0, sizeof(struct nlattr *) * (maxtype + 1));
	nla_for_each_attr(nla, head, len, rem) {
		u16 type = nla_type(nla);
		if (type == 0 || type > maxtype) {
			if (validate & NL_VALIDATE_MAXTYPE) {
				NL_SET_ERR_MSG_ATTR(extack, nla,
						    "Unknown attribute type");
				return -EINVAL;
			}
			continue;
		}
		if (policy) {
			int err = validate_nla(nla, maxtype, policy,
					       validate, extack);
			if (err < 0)
				return err;
		}
		if (tb)
			tb[type] = (struct nlattr *)nla;
	}
	if (unlikely(rem > 0)) {
		pr_warn_ratelimited("netlink: %d bytes leftover after parsing attributes in process `%s'.\n",
				    rem, current->comm);
		NL_SET_ERR_MSG(extack, "bytes leftover after parsing attributes");
		if (validate & NL_VALIDATE_TRAILING)
			return -EINVAL;
	}
	return 0;
}