do_core_note(struct magic_set *ms, unsigned char *nbuf, uint32_t type,
    int swap, uint32_t namesz, uint32_t descsz,
    size_t noff, size_t doff, int *flags, size_t size, int clazz)
{
#ifdef ELFCORE
	int os_style = -1;
	if ((namesz == 4 && strncmp((char *)&nbuf[noff], "CORE", 4) == 0) ||
	    (namesz == 5 && strcmp((char *)&nbuf[noff], "CORE") == 0)) {
		os_style = OS_STYLE_SVR4;
	}
	if ((namesz == 8 && strcmp((char *)&nbuf[noff], "FreeBSD") == 0)) {
		os_style = OS_STYLE_FREEBSD;
	}
	if ((namesz >= 11 && strncmp((char *)&nbuf[noff], "NetBSD-CORE", 11)
	    == 0)) {
		os_style = OS_STYLE_NETBSD;
	}
	if (os_style != -1 && (*flags & FLAGS_DID_CORE_STYLE) == 0) {
		if (file_printf(ms, ", %s-style", os_style_names[os_style])
		    == -1)
			return 1;
		*flags |= FLAGS_DID_CORE_STYLE;
		*flags |= os_style;
	}
	switch (os_style) {
	case OS_STYLE_NETBSD:
		if (type == NT_NETBSD_CORE_PROCINFO) {
			char sbuf[512];
			struct NetBSD_elfcore_procinfo pi;
			memset(&pi, 0, sizeof(pi));
			memcpy(&pi, nbuf + doff, descsz);
			if (file_printf(ms, ", from '%.31s', pid=%u, uid=%u, "
			    "gid=%u, nlwps=%u, lwp=%u (signal %u/code %u)",
			    file_printable(sbuf, sizeof(sbuf),
			    RCAST(char *, pi.cpi_name)),
			    elf_getu32(swap, (uint32_t)pi.cpi_pid),
			    elf_getu32(swap, pi.cpi_euid),
			    elf_getu32(swap, pi.cpi_egid),
			    elf_getu32(swap, pi.cpi_nlwps),
			    elf_getu32(swap, (uint32_t)pi.cpi_siglwp),
			    elf_getu32(swap, pi.cpi_signo),
			    elf_getu32(swap, pi.cpi_sigcode)) == -1)
				return 1;
			*flags |= FLAGS_DID_CORE;
			return 1;
		}
		break;
	case OS_STYLE_FREEBSD:
		if (type == NT_PRPSINFO && *flags & FLAGS_IS_CORE) {
			size_t argoff, pidoff;
			if (clazz == ELFCLASS32)
				argoff = 4 + 4 + 17;
			else
				argoff = 4 + 4 + 8 + 17;
			if (file_printf(ms, ", from '%.80s'", nbuf + doff +
			    argoff) == -1)
				return 1;
			pidoff = argoff + 81 + 2;
			if (doff + pidoff + 4 <= size) {
				if (file_printf(ms, ", pid=%u",
				    elf_getu32(swap, *RCAST(uint32_t *, (nbuf +
				    doff + pidoff)))) == -1)
					return 1;
			}
			*flags |= FLAGS_DID_CORE;
		}			    
		break;
	default:
		if (type == NT_PRPSINFO && *flags & FLAGS_IS_CORE) {
			size_t i, j;
			unsigned char c;
			for (i = 0; i < NOFFSETS; i++) {
				unsigned char *cname, *cp;
				size_t reloffset = prpsoffsets(i);
				size_t noffset = doff + reloffset;
				size_t k;
				for (j = 0; j < 16; j++, noffset++,
				    reloffset++) {
					if (noffset >= size)
						goto tryanother;
					if (reloffset >= descsz)
						goto tryanother;
					c = nbuf[noffset];
					if (c == '\0') {
						if (j == 0)
							goto tryanother;
						else
							break;
					} else {
						if (!isprint(c) || isquote(c))
							goto tryanother;
					}
				}
				for (k = i + 1 ; k < NOFFSETS; k++) {
					size_t no;
					int adjust = 1;
					if (prpsoffsets(k) >= prpsoffsets(i))
						continue;
					for (no = doff + prpsoffsets(k);
					     no < doff + prpsoffsets(i); no++)
						adjust = adjust
						         && isprint(nbuf[no]);
					if (adjust)
						i = k;
				}
				cname = (unsigned char *)
				    &nbuf[doff + prpsoffsets(i)];
				for (cp = cname; cp < nbuf + size && *cp
				    && isprint(*cp); cp++)
					continue;
				while (cp > cname && isspace(cp[-1]))
					cp--;
				if (file_printf(ms, ", from '%.*s'",
				    (int)(cp - cname), cname) == -1)
					return 1;
				*flags |= FLAGS_DID_CORE;
				return 1;
			tryanother:
				;
			}
		}
		break;
	}
#endif
	return 0;
}