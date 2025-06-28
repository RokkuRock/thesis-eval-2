donote(struct magic_set *ms, void *vbuf, size_t offset, size_t size,
    int clazz, int swap, size_t align, int *flags)
{
	Elf32_Nhdr nh32;
	Elf64_Nhdr nh64;
	size_t noff, doff;
#ifdef ELFCORE
	int os_style = -1;
#endif
	uint32_t namesz, descsz;
	unsigned char *nbuf = CAST(unsigned char *, vbuf);
	if (xnh_sizeof + offset > size) {
		return xnh_sizeof + offset;
	}
	(void)memcpy(xnh_addr, &nbuf[offset], xnh_sizeof);
	offset += xnh_sizeof;
	namesz = xnh_namesz;
	descsz = xnh_descsz;
	if ((namesz == 0) && (descsz == 0)) {
		return (offset >= size) ? offset : size;
	}
	if (namesz & 0x80000000) {
	    (void)file_printf(ms, ", bad note name size 0x%lx",
		(unsigned long)namesz);
	    return offset;
	}
	if (descsz & 0x80000000) {
	    (void)file_printf(ms, ", bad note description size 0x%lx",
		(unsigned long)descsz);
	    return offset;
	}
	noff = offset;
	doff = ELF_ALIGN(offset + namesz);
	if (offset + namesz > size) {
		return doff;
	}
	offset = ELF_ALIGN(doff + descsz);
	if (doff + descsz > size) {
		return (offset >= size) ? offset : size;
	}
	if ((*flags & (FLAGS_DID_NOTE|FLAGS_DID_BUILD_ID)) ==
	    (FLAGS_DID_NOTE|FLAGS_DID_BUILD_ID))
		goto core;
	if (namesz == 5 && strcmp((char *)&nbuf[noff], "SuSE") == 0 &&
	    xnh_type == NT_GNU_VERSION && descsz == 2) {
	    file_printf(ms, ", for SuSE %d.%d", nbuf[doff], nbuf[doff + 1]);
	}
	if (namesz == 4 && strcmp((char *)&nbuf[noff], "GNU") == 0 &&
	    xnh_type == NT_GNU_VERSION && descsz == 16) {
		uint32_t desc[4];
		(void)memcpy(desc, &nbuf[doff], sizeof(desc));
		if (file_printf(ms, ", for GNU/") == -1)
			return size;
		switch (elf_getu32(swap, desc[0])) {
		case GNU_OS_LINUX:
			if (file_printf(ms, "Linux") == -1)
				return size;
			break;
		case GNU_OS_HURD:
			if (file_printf(ms, "Hurd") == -1)
				return size;
			break;
		case GNU_OS_SOLARIS:
			if (file_printf(ms, "Solaris") == -1)
				return size;
			break;
		case GNU_OS_KFREEBSD:
			if (file_printf(ms, "kFreeBSD") == -1)
				return size;
			break;
		case GNU_OS_KNETBSD:
			if (file_printf(ms, "kNetBSD") == -1)
				return size;
			break;
		default:
			if (file_printf(ms, "<unknown>") == -1)
				return size; 
		}
		if (file_printf(ms, " %d.%d.%d", elf_getu32(swap, desc[1]),
		    elf_getu32(swap, desc[2]), elf_getu32(swap, desc[3])) == -1)
			return size;
		*flags |= FLAGS_DID_NOTE;
		return size;
	}
	if (namesz == 4 && strcmp((char *)&nbuf[noff], "GNU") == 0 &&
	    xnh_type == NT_GNU_BUILD_ID && (descsz == 16 || descsz == 20)) {
	    uint8_t desc[20];
	    uint32_t i;
	    if (file_printf(ms, ", BuildID[%s]=", descsz == 16 ? "md5/uuid" :
		"sha1") == -1)
		    return size;
	    (void)memcpy(desc, &nbuf[doff], descsz);
	    for (i = 0; i < descsz; i++)
		if (file_printf(ms, "%02x", desc[i]) == -1)
		    return size;
	    *flags |= FLAGS_DID_BUILD_ID;
	}
	if (namesz == 4 && strcmp((char *)&nbuf[noff], "PaX") == 0 &&
	    xnh_type == NT_NETBSD_PAX && descsz == 4) {
		static const char *pax[] = {
		    "+mprotect",
		    "-mprotect",
		    "+segvguard",
		    "-segvguard",
		    "+ASLR",
		    "-ASLR",
		};
		uint32_t desc;
		size_t i;
		int did = 0;
		(void)memcpy(&desc, &nbuf[doff], sizeof(desc));
		desc = elf_getu32(swap, desc);
		if (desc && file_printf(ms, ", PaX: ") == -1)
			return size;
		for (i = 0; i < __arraycount(pax); i++) {
			if (((1 << i) & desc) == 0)
				continue;
			if (file_printf(ms, "%s%s", did++ ? "," : "",
			    pax[i]) == -1)
				return size;
		}
	}
	if (namesz == 7 && strcmp((char *)&nbuf[noff], "NetBSD") == 0) {
		switch (xnh_type) {
		case NT_NETBSD_VERSION:
			if (descsz == 4) {
				do_note_netbsd_version(ms, swap, &nbuf[doff]);
				*flags |= FLAGS_DID_NOTE;
				return size;
			}
			break;
		case NT_NETBSD_MARCH:
			if (file_printf(ms, ", compiled for: %.*s", (int)descsz,
			    (const char *)&nbuf[doff]) == -1)
				return size;
			break;
		case NT_NETBSD_CMODEL:
			if (file_printf(ms, ", compiler model: %.*s",
			    (int)descsz, (const char *)&nbuf[doff]) == -1)
				return size;
			break;
		default:
			if (file_printf(ms, ", note=%u", xnh_type) == -1)
				return size;
			break;
		}
		return size;
	}
	if (namesz == 8 && strcmp((char *)&nbuf[noff], "FreeBSD") == 0) {
	    	if (xnh_type == NT_FREEBSD_VERSION && descsz == 4) {
			do_note_freebsd_version(ms, swap, &nbuf[doff]);
			*flags |= FLAGS_DID_NOTE;
			return size;
		}
	}
	if (namesz == 8 && strcmp((char *)&nbuf[noff], "OpenBSD") == 0 &&
	    xnh_type == NT_OPENBSD_VERSION && descsz == 4) {
		if (file_printf(ms, ", for OpenBSD") == -1)
			return size;
		*flags |= FLAGS_DID_NOTE;
		return size;
	}
	if (namesz == 10 && strcmp((char *)&nbuf[noff], "DragonFly") == 0 &&
	    xnh_type == NT_DRAGONFLY_VERSION && descsz == 4) {
		uint32_t desc;
		if (file_printf(ms, ", for DragonFly") == -1)
			return size;
		(void)memcpy(&desc, &nbuf[doff], sizeof(desc));
		desc = elf_getu32(swap, desc);
		if (file_printf(ms, " %d.%d.%d", desc / 100000,
		    desc / 10000 % 10, desc % 10000) == -1)
			return size;
		*flags |= FLAGS_DID_NOTE;
		return size;
	}
core:
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
#ifdef ELFCORE
	if ((*flags & FLAGS_DID_CORE) != 0)
		return size;
	if (os_style != -1 && (*flags & FLAGS_DID_CORE_STYLE) == 0) {
		if (file_printf(ms, ", %s-style", os_style_names[os_style])
		    == -1)
			return size;
		*flags |= FLAGS_DID_CORE_STYLE;
	}
	switch (os_style) {
	case OS_STYLE_NETBSD:
		if (xnh_type == NT_NETBSD_CORE_PROCINFO) {
			uint32_t signo;
			if (file_printf(ms, ", from '%.31s'",
			    &nbuf[doff + 0x7c]) == -1)
				return size;
			(void)memcpy(&signo, &nbuf[doff + 0x08],
			    sizeof(signo));
			if (file_printf(ms, " (signal %u)",
			    elf_getu32(swap, signo)) == -1)
				return size;
			*flags |= FLAGS_DID_CORE;
			return size;
		}
		break;
	default:
		if (xnh_type == NT_PRPSINFO && *flags & FLAGS_IS_CORE) {
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
				for (k = i + 1 ; k < NOFFSETS ; k++) {
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
				for (cp = cname; *cp && isprint(*cp); cp++)
					continue;
				while (cp > cname && isspace(cp[-1]))
					cp--;
				if (file_printf(ms, ", from '%.*s'",
				    (int)(cp - cname), cname) == -1)
					return size;
				*flags |= FLAGS_DID_CORE;
				return size;
			tryanother:
				;
			}
		}
		break;
	}
#endif
	return offset;
}