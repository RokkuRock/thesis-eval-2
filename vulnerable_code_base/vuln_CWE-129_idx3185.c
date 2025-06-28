dumpppp(f)
    FILE *f;
{
    int c, n, k;
    int nb, nl, dn, proto, rv;
    char *dir, *q;
    unsigned char *p, *r, *endp;
    unsigned char *d;
    unsigned short fcs;
    struct pkt *pkt;
    spkt.cnt = rpkt.cnt = 0;
    spkt.esc = rpkt.esc = 0;
    while ((c = getc(f)) != EOF) {
	switch (c) {
	case 1:
	case 2:
	    if (reverse)
		c = 3 - c;
	    dir = c==1? "sent": "rcvd";
	    pkt = c==1? &spkt: &rpkt;
	    n = getc(f);
	    n = (n << 8) + getc(f);
	    *(c==1? &tot_sent: &tot_rcvd) += n;
	    for (; n > 0; --n) {
		c = getc(f);
		switch (c) {
		case EOF:
		    printf("\nEOF\n");
		    if (spkt.cnt > 0)
			printf("[%d bytes in incomplete send packet]\n",
			       spkt.cnt);
		    if (rpkt.cnt > 0)
			printf("[%d bytes in incomplete recv packet]\n",
			       rpkt.cnt);
		    exit(0);
		case '~':
		    if (pkt->cnt > 0) {
			q = dir;
			if (pkt->esc) {
			    printf("%s aborted packet:\n     ", dir);
			    q = "    ";
			}
			nb = pkt->cnt;
			p = pkt->buf;
			pkt->cnt = 0;
			pkt->esc = 0;
			if (nb <= 2) {
			    printf("%s short packet [%d bytes]:", q, nb);
			    for (k = 0; k < nb; ++k)
				printf(" %.2x", p[k]);
			    printf("\n");
			    break;
			}
			fcs = PPP_INITFCS;
			for (k = 0; k < nb; ++k)
			    fcs = PPP_FCS(fcs, p[k]);
			fcs &= 0xFFFF;
			nb -= 2;
			endp = p + nb;
			r = p;
			if (r[0] == 0xff && r[1] == 3)
			    r += 2;
			if ((r[0] & 1) == 0)
			    ++r;
			++r;
			if (endp - r > mru)
			    printf("     ERROR: length (%zd) > MRU (%d)\n",
				   endp - r, mru);
			if (decompress && fcs == PPP_GOODFCS) {
			    d = dbuf;
			    r = p;
			    if (r[0] == 0xff && r[1] == 3) {
				*d++ = *r++;
				*d++ = *r++;
			    }
			    proto = r[0];
			    if ((proto & 1) == 0)
				proto = (proto << 8) + r[1];
			    if (proto == PPP_CCP) {
				handle_ccp(pkt, r + 2, endp - r - 2);
			    } else if (proto == PPP_COMP) {
				if ((pkt->flags & CCP_ISUP)
				    && (pkt->flags & CCP_DECOMP_RUN)
				    && pkt->state
				    && (pkt->flags & CCP_ERR) == 0) {
				    rv = pkt->comp->decompress(pkt->state, r,
							endp - r, d, &dn);
				    switch (rv) {
				    case DECOMP_OK:
					p = dbuf;
					nb = d + dn - p;
					if ((d[0] & 1) == 0)
					    --dn;
					--dn;
					if (dn > mru)
					    printf("     ERROR: decompressed length (%d) > MRU (%d)\n", dn, mru);
					break;
				    case DECOMP_ERROR:
					printf("     DECOMPRESSION ERROR\n");
					pkt->flags |= CCP_ERROR;
					break;
				    case DECOMP_FATALERROR:
					printf("     FATAL DECOMPRESSION ERROR\n");
					pkt->flags |= CCP_FATALERROR;
					break;
				    }
				}
			    } else if (pkt->state
				       && (pkt->flags & CCP_DECOMP_RUN)) {
				pkt->comp->incomp(pkt->state, r, endp - r);
			    }
			}
			do {
			    nl = nb < 16? nb: 16;
			    printf("%s ", q);
			    for (k = 0; k < nl; ++k)
				printf(" %.2x", p[k]);
			    for (; k < 16; ++k)
				printf("   ");
			    printf("  ");
			    for (k = 0; k < nl; ++k) {
				c = p[k];
				putchar((' ' <= c && c <= '~')? c: '.');
			    }
			    printf("\n");
			    q = "    ";
			    p += nl;
			    nb -= nl;
			} while (nb > 0);
			if (fcs != PPP_GOODFCS)
			    printf("     BAD FCS: (residue = %x)\n", fcs);
		    }
		    break;
		case '}':
		    if (!pkt->esc) {
			pkt->esc = 1;
			break;
		    }
		default:
		    if (pkt->esc) {
			c ^= 0x20;
			pkt->esc = 0;
		    }
		    pkt->buf[pkt->cnt++] = c;
		    break;
		}
	    }
	    break;
	case 3:
	case 4:
	    if (reverse)
		c = 7 - c;
	    dir = c==3? "send": "recv";
	    pkt = c==3? &spkt: &rpkt;
	    printf("end %s", dir);
	    if (pkt->cnt > 0)
		printf("  [%d bytes in incomplete packet]", pkt->cnt);
	    printf("\n");
	    break;
	case 5:
	case 6:
	case 7:
	    show_time(f, c);
	    break;
	default:
	    printf("?%.2x\n", c);
	}
    }
}