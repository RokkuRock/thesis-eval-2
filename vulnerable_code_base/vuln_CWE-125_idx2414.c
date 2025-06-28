mpls_print(netdissect_options *ndo, const u_char *bp, u_int length)
{
	const u_char *p;
	uint32_t label_entry;
	uint16_t label_stack_depth = 0;
	enum mpls_packet_type pt = PT_UNKNOWN;
	p = bp;
	ND_PRINT((ndo, "MPLS"));
	do {
		ND_TCHECK2(*p, sizeof(label_entry));
		if (length < sizeof(label_entry)) {
			ND_PRINT((ndo, "[|MPLS], length %u", length));
			return;
		}
		label_entry = EXTRACT_32BITS(p);
		ND_PRINT((ndo, "%s(label %u",
		       (label_stack_depth && ndo->ndo_vflag) ? "\n\t" : " ",
       		       MPLS_LABEL(label_entry)));
		label_stack_depth++;
		if (ndo->ndo_vflag &&
		    MPLS_LABEL(label_entry) < sizeof(mpls_labelname) / sizeof(mpls_labelname[0]))
			ND_PRINT((ndo, " (%s)", mpls_labelname[MPLS_LABEL(label_entry)]));
		ND_PRINT((ndo, ", exp %u", MPLS_EXP(label_entry)));
		if (MPLS_STACK(label_entry))
			ND_PRINT((ndo, ", [S]"));
		ND_PRINT((ndo, ", ttl %u)", MPLS_TTL(label_entry)));
		p += sizeof(label_entry);
		length -= sizeof(label_entry);
	} while (!MPLS_STACK(label_entry));
	switch (MPLS_LABEL(label_entry)) {
	case 0:	 
	case 3:	 
		pt = PT_IPV4;
		break;
	case 2:	 
		pt = PT_IPV6;
		break;
	default:
		ND_TCHECK(*p);
		if (length < 1) {
			return;
		}
		switch(*p) {
		case 0x45:
		case 0x46:
		case 0x47:
		case 0x48:
		case 0x49:
		case 0x4a:
		case 0x4b:
		case 0x4c:
		case 0x4d:
		case 0x4e:
		case 0x4f:
			pt = PT_IPV4;
			break;
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
		case 0x68:
		case 0x69:
		case 0x6a:
		case 0x6b:
		case 0x6c:
		case 0x6d:
		case 0x6e:
		case 0x6f:
			pt = PT_IPV6;
			break;
		case 0x81:
		case 0x82:
		case 0x83:
			pt = PT_OSI;
			break;
		default:
			break;
		}
	}
	if (pt == PT_UNKNOWN) {
		if (!ndo->ndo_suppress_default_print)
			ND_DEFAULTPRINT(p, length);
		return;
	}
	ND_PRINT((ndo, ndo->ndo_vflag ? "\n\t" : " "));
	switch (pt) {
	case PT_IPV4:
		ip_print(ndo, p, length);
		break;
	case PT_IPV6:
		ip6_print(ndo, p, length);
		break;
	case PT_OSI:
		isoclns_print(ndo, p, length, length);
		break;
	default:
		break;
	}
	return;
trunc:
	ND_PRINT((ndo, "[|MPLS]"));
}