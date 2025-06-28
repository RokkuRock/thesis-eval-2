bgp_attr_parse_ret_t bgp_attr_parse(struct peer *peer, struct attr *attr,
				    bgp_size_t size, struct bgp_nlri *mp_update,
				    struct bgp_nlri *mp_withdraw)
{
	bgp_attr_parse_ret_t ret;
	uint8_t flag = 0;
	uint8_t type = 0;
	bgp_size_t length;
	uint8_t *startp, *endp;
	uint8_t *attr_endp;
	uint8_t seen[BGP_ATTR_BITMAP_SIZE];
	struct aspath *as4_path = NULL;
	as_t as4_aggregator = 0;
	struct in_addr as4_aggregator_addr = {.s_addr = 0};
	memset(seen, 0, BGP_ATTR_BITMAP_SIZE);
	endp = BGP_INPUT_PNT(peer) + size;
	while (BGP_INPUT_PNT(peer) < endp) {
		if (endp - BGP_INPUT_PNT(peer) < BGP_ATTR_MIN_LEN) {
			flog_warn(
				EC_BGP_ATTRIBUTE_TOO_SMALL,
				"%s: error BGP attribute length %lu is smaller than min len",
				peer->host,
				(unsigned long)(endp
						- stream_pnt(BGP_INPUT(peer))));
			bgp_notify_send(peer, BGP_NOTIFY_UPDATE_ERR,
					BGP_NOTIFY_UPDATE_ATTR_LENG_ERR);
			return BGP_ATTR_PARSE_ERROR;
		}
		startp = BGP_INPUT_PNT(peer);
		flag = 0xF0 & stream_getc(BGP_INPUT(peer));
		type = stream_getc(BGP_INPUT(peer));
		if (CHECK_FLAG(flag, BGP_ATTR_FLAG_EXTLEN)
		    && ((endp - startp) < (BGP_ATTR_MIN_LEN + 1))) {
			flog_warn(
				EC_BGP_EXT_ATTRIBUTE_TOO_SMALL,
				"%s: Extended length set, but just %lu bytes of attr header",
				peer->host,
				(unsigned long)(endp
						- stream_pnt(BGP_INPUT(peer))));
			bgp_notify_send(peer, BGP_NOTIFY_UPDATE_ERR,
					BGP_NOTIFY_UPDATE_ATTR_LENG_ERR);
			return BGP_ATTR_PARSE_ERROR;
		}
		if (CHECK_FLAG(flag, BGP_ATTR_FLAG_EXTLEN))
			length = stream_getw(BGP_INPUT(peer));
		else
			length = stream_getc(BGP_INPUT(peer));
		if (CHECK_BITMAP(seen, type)) {
			flog_warn(
				EC_BGP_ATTRIBUTE_REPEATED,
				"%s: error BGP attribute type %d appears twice in a message",
				peer->host, type);
			bgp_notify_send(peer, BGP_NOTIFY_UPDATE_ERR,
					BGP_NOTIFY_UPDATE_MAL_ATTR);
			return BGP_ATTR_PARSE_ERROR;
		}
		SET_BITMAP(seen, type);
		attr_endp = BGP_INPUT_PNT(peer) + length;
		if (attr_endp > endp) {
			flog_warn(
				EC_BGP_ATTRIBUTE_TOO_LARGE,
				"%s: BGP type %d length %d is too large, attribute total length is %d.  attr_endp is %p.  endp is %p",
				peer->host, type, length, size, attr_endp,
				endp);
			unsigned char ndata[BGP_MAX_PACKET_SIZE];
			memset(ndata, 0x00, sizeof(ndata));
			size_t lfl =
				CHECK_FLAG(flag, BGP_ATTR_FLAG_EXTLEN) ? 2 : 1;
			stream_forward_getp(BGP_INPUT(peer), -(1 + lfl));
			stream_get(&ndata[0], BGP_INPUT(peer), 1);
			stream_get(&ndata[1], BGP_INPUT(peer), lfl);
			size_t atl = attr_endp - startp;
			size_t ndl = MIN(atl, STREAM_READABLE(BGP_INPUT(peer)));
			stream_get(&ndata[lfl + 1], BGP_INPUT(peer), ndl);
			bgp_notify_send_with_data(
				peer, BGP_NOTIFY_UPDATE_ERR,
				BGP_NOTIFY_UPDATE_ATTR_LENG_ERR, ndata,
				ndl + lfl + 1);
			return BGP_ATTR_PARSE_ERROR;
		}
		struct bgp_attr_parser_args attr_args = {
			.peer = peer,
			.length = length,
			.attr = attr,
			.type = type,
			.flags = flag,
			.startp = startp,
			.total = attr_endp - startp,
		};
		if (bgp_attr_flag_invalid(&attr_args)) {
			ret = bgp_attr_malformed(
				&attr_args, BGP_NOTIFY_UPDATE_ATTR_FLAG_ERR,
				attr_args.total);
			if (ret == BGP_ATTR_PARSE_PROCEED)
				continue;
			return ret;
		}
		switch (type) {
		case BGP_ATTR_ORIGIN:
			ret = bgp_attr_origin(&attr_args);
			break;
		case BGP_ATTR_AS_PATH:
			ret = bgp_attr_aspath(&attr_args);
			break;
		case BGP_ATTR_AS4_PATH:
			ret = bgp_attr_as4_path(&attr_args, &as4_path);
			break;
		case BGP_ATTR_NEXT_HOP:
			ret = bgp_attr_nexthop(&attr_args);
			break;
		case BGP_ATTR_MULTI_EXIT_DISC:
			ret = bgp_attr_med(&attr_args);
			break;
		case BGP_ATTR_LOCAL_PREF:
			ret = bgp_attr_local_pref(&attr_args);
			break;
		case BGP_ATTR_ATOMIC_AGGREGATE:
			ret = bgp_attr_atomic(&attr_args);
			break;
		case BGP_ATTR_AGGREGATOR:
			ret = bgp_attr_aggregator(&attr_args);
			break;
		case BGP_ATTR_AS4_AGGREGATOR:
			ret = bgp_attr_as4_aggregator(&attr_args,
						      &as4_aggregator,
						      &as4_aggregator_addr);
			break;
		case BGP_ATTR_COMMUNITIES:
			ret = bgp_attr_community(&attr_args);
			break;
		case BGP_ATTR_LARGE_COMMUNITIES:
			ret = bgp_attr_large_community(&attr_args);
			break;
		case BGP_ATTR_ORIGINATOR_ID:
			ret = bgp_attr_originator_id(&attr_args);
			break;
		case BGP_ATTR_CLUSTER_LIST:
			ret = bgp_attr_cluster_list(&attr_args);
			break;
		case BGP_ATTR_MP_REACH_NLRI:
			ret = bgp_mp_reach_parse(&attr_args, mp_update);
			break;
		case BGP_ATTR_MP_UNREACH_NLRI:
			ret = bgp_mp_unreach_parse(&attr_args, mp_withdraw);
			break;
		case BGP_ATTR_EXT_COMMUNITIES:
			ret = bgp_attr_ext_communities(&attr_args);
			break;
#if ENABLE_BGP_VNC
		case BGP_ATTR_VNC:
#endif
		case BGP_ATTR_ENCAP:
			ret = bgp_attr_encap(type, peer, length, attr, flag,
					     startp);
			break;
		case BGP_ATTR_PREFIX_SID:
			ret = bgp_attr_prefix_sid(length,
						  &attr_args, mp_update);
			break;
		case BGP_ATTR_PMSI_TUNNEL:
			ret = bgp_attr_pmsi_tunnel(&attr_args);
			break;
		default:
			ret = bgp_attr_unknown(&attr_args);
			break;
		}
		if (ret == BGP_ATTR_PARSE_ERROR_NOTIFYPLS) {
			bgp_notify_send(peer, BGP_NOTIFY_UPDATE_ERR,
					BGP_NOTIFY_UPDATE_MAL_ATTR);
			ret = BGP_ATTR_PARSE_ERROR;
		}
		if (ret == BGP_ATTR_PARSE_EOR) {
			if (as4_path)
				aspath_unintern(&as4_path);
			return ret;
		}
		if (ret == BGP_ATTR_PARSE_ERROR) {
			flog_warn(EC_BGP_ATTRIBUTE_PARSE_ERROR,
				  "%s: Attribute %s, parse error", peer->host,
				  lookup_msg(attr_str, type, NULL));
			if (as4_path)
				aspath_unintern(&as4_path);
			return ret;
		}
		if (ret == BGP_ATTR_PARSE_WITHDRAW) {
			flog_warn(
				EC_BGP_ATTRIBUTE_PARSE_WITHDRAW,
				"%s: Attribute %s, parse error - treating as withdrawal",
				peer->host, lookup_msg(attr_str, type, NULL));
			if (as4_path)
				aspath_unintern(&as4_path);
			return ret;
		}
		if (BGP_INPUT_PNT(peer) != attr_endp) {
			flog_warn(EC_BGP_ATTRIBUTE_FETCH_ERROR,
				  "%s: BGP attribute %s, fetch error",
				  peer->host, lookup_msg(attr_str, type, NULL));
			bgp_notify_send(peer, BGP_NOTIFY_UPDATE_ERR,
					BGP_NOTIFY_UPDATE_ATTR_LENG_ERR);
			if (as4_path)
				aspath_unintern(&as4_path);
			return BGP_ATTR_PARSE_ERROR;
		}
	}
	if (BGP_INPUT_PNT(peer) != endp) {
		flog_warn(EC_BGP_ATTRIBUTES_MISMATCH,
			  "%s: BGP attribute %s, length mismatch", peer->host,
			  lookup_msg(attr_str, type, NULL));
		bgp_notify_send(peer, BGP_NOTIFY_UPDATE_ERR,
				BGP_NOTIFY_UPDATE_ATTR_LENG_ERR);
		if (as4_path)
			aspath_unintern(&as4_path);
		return BGP_ATTR_PARSE_ERROR;
	}
	if ((ret = bgp_attr_check(peer, attr)) < 0) {
		if (as4_path)
			aspath_unintern(&as4_path);
		return ret;
	}
	if (CHECK_FLAG(attr->flag, ATTR_FLAG_BIT(BGP_ATTR_AS_PATH))
	    && bgp_attr_munge_as4_attrs(peer, attr, as4_path, as4_aggregator,
					&as4_aggregator_addr)) {
		bgp_notify_send(peer, BGP_NOTIFY_UPDATE_ERR,
				BGP_NOTIFY_UPDATE_MAL_ATTR);
		if (as4_path)
			aspath_unintern(&as4_path);
		return BGP_ATTR_PARSE_ERROR;
	}
	if (as4_path) {
		aspath_unintern(&as4_path);  
	}
	if (attr->flag & (ATTR_FLAG_BIT(BGP_ATTR_AS_PATH))) {
		ret = bgp_attr_aspath_check(peer, attr);
		if (ret != BGP_ATTR_PARSE_PROCEED)
			return ret;
	}
	if (attr->transit)
		attr->transit = transit_intern(attr->transit);
	if (attr->encap_subtlvs)
		attr->encap_subtlvs =
			encap_intern(attr->encap_subtlvs, ENCAP_SUBTLV_TYPE);
#if ENABLE_BGP_VNC
	if (attr->vnc_subtlvs)
		attr->vnc_subtlvs =
			encap_intern(attr->vnc_subtlvs, VNC_SUBTLV_TYPE);
#endif
	return BGP_ATTR_PARSE_PROCEED;
}