static int validate_nla(const struct nlattr *nla, int maxtype,
			const struct nla_policy *policy, unsigned int validate,
			struct netlink_ext_ack *extack)
{
	u16 strict_start_type = policy[0].strict_start_type;
	const struct nla_policy *pt;
	int minlen = 0, attrlen = nla_len(nla), type = nla_type(nla);
	int err = -ERANGE;
	if (strict_start_type && type >= strict_start_type)
		validate |= NL_VALIDATE_STRICT;
	if (type <= 0 || type > maxtype)
		return 0;
	pt = &policy[type];
	BUG_ON(pt->type > NLA_TYPE_MAX);
	if ((nla_attr_len[pt->type] && attrlen != nla_attr_len[pt->type]) ||
	    (pt->type == NLA_EXACT_LEN_WARN && attrlen != pt->len)) {
		pr_warn_ratelimited("netlink: '%s': attribute type %d has an invalid length.\n",
				    current->comm, type);
		if (validate & NL_VALIDATE_STRICT_ATTRS) {
			NL_SET_ERR_MSG_ATTR(extack, nla,
					    "invalid attribute length");
			return -EINVAL;
		}
	}
	if (validate & NL_VALIDATE_NESTED) {
		if ((pt->type == NLA_NESTED || pt->type == NLA_NESTED_ARRAY) &&
		    !(nla->nla_type & NLA_F_NESTED)) {
			NL_SET_ERR_MSG_ATTR(extack, nla,
					    "NLA_F_NESTED is missing");
			return -EINVAL;
		}
		if (pt->type != NLA_NESTED && pt->type != NLA_NESTED_ARRAY &&
		    pt->type != NLA_UNSPEC && (nla->nla_type & NLA_F_NESTED)) {
			NL_SET_ERR_MSG_ATTR(extack, nla,
					    "NLA_F_NESTED not expected");
			return -EINVAL;
		}
	}
	switch (pt->type) {
	case NLA_EXACT_LEN:
		if (attrlen != pt->len)
			goto out_err;
		break;
	case NLA_REJECT:
		if (extack && pt->reject_message) {
			NL_SET_BAD_ATTR(extack, nla);
			extack->_msg = pt->reject_message;
			return -EINVAL;
		}
		err = -EINVAL;
		goto out_err;
	case NLA_FLAG:
		if (attrlen > 0)
			goto out_err;
		break;
	case NLA_BITFIELD32:
		if (attrlen != sizeof(struct nla_bitfield32))
			goto out_err;
		err = validate_nla_bitfield32(nla, pt->bitfield32_valid);
		if (err)
			goto out_err;
		break;
	case NLA_NUL_STRING:
		if (pt->len)
			minlen = min_t(int, attrlen, pt->len + 1);
		else
			minlen = attrlen;
		if (!minlen || memchr(nla_data(nla), '\0', minlen) == NULL) {
			err = -EINVAL;
			goto out_err;
		}
	case NLA_STRING:
		if (attrlen < 1)
			goto out_err;
		if (pt->len) {
			char *buf = nla_data(nla);
			if (buf[attrlen - 1] == '\0')
				attrlen--;
			if (attrlen > pt->len)
				goto out_err;
		}
		break;
	case NLA_BINARY:
		if (pt->len && attrlen > pt->len)
			goto out_err;
		break;
	case NLA_NESTED:
		if (attrlen == 0)
			break;
		if (attrlen < NLA_HDRLEN)
			goto out_err;
		if (pt->nested_policy) {
			err = __nla_validate(nla_data(nla), nla_len(nla), pt->len,
					     pt->nested_policy, validate,
					     extack);
			if (err < 0) {
				return err;
			}
		}
		break;
	case NLA_NESTED_ARRAY:
		if (attrlen == 0)
			break;
		if (attrlen < NLA_HDRLEN)
			goto out_err;
		if (pt->nested_policy) {
			int err;
			err = nla_validate_array(nla_data(nla), nla_len(nla),
						 pt->len, pt->nested_policy,
						 extack, validate);
			if (err < 0) {
				return err;
			}
		}
		break;
	case NLA_UNSPEC:
		if (validate & NL_VALIDATE_UNSPEC) {
			NL_SET_ERR_MSG_ATTR(extack, nla,
					    "Unsupported attribute");
			return -EINVAL;
		}
	case NLA_MIN_LEN:
		if (attrlen < pt->len)
			goto out_err;
		break;
	default:
		if (pt->len)
			minlen = pt->len;
		else
			minlen = nla_attr_minlen[pt->type];
		if (attrlen < minlen)
			goto out_err;
	}
	switch (pt->validation_type) {
	case NLA_VALIDATE_NONE:
		break;
	case NLA_VALIDATE_RANGE:
	case NLA_VALIDATE_MIN:
	case NLA_VALIDATE_MAX:
		err = nla_validate_int_range(pt, nla, extack);
		if (err)
			return err;
		break;
	case NLA_VALIDATE_FUNCTION:
		if (pt->validate) {
			err = pt->validate(nla, extack);
			if (err)
				return err;
		}
		break;
	}
	return 0;
out_err:
	NL_SET_ERR_MSG_ATTR(extack, nla, "Attribute failed policy validation");
	return err;
}