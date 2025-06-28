cleanup_pathname(struct archive_write_disk *a)
{
	char *dest, *src;
	char separator = '\0';
	dest = src = a->name;
	if (*src == '\0') {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
		    "Invalid empty pathname");
		return (ARCHIVE_FAILED);
	}
#if defined(__CYGWIN__)
	cleanup_pathname_win(a);
#endif
	if (*src == '/') {
		if (a->flags & ARCHIVE_EXTRACT_SECURE_NOABSOLUTEPATHS) {
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			                  "Path is absolute");
			return (ARCHIVE_FAILED);
		}
		separator = *src++;
	}
	for (;;) {
		if (src[0] == '\0') {
			break;
		} else if (src[0] == '/') {
			src++;
			continue;
		} else if (src[0] == '.') {
			if (src[1] == '\0') {
				break;
			} else if (src[1] == '/') {
				src += 2;
				continue;
			} else if (src[1] == '.') {
				if (src[2] == '/' || src[2] == '\0') {
					if (a->flags & ARCHIVE_EXTRACT_SECURE_NODOTDOT) {
						archive_set_error(&a->archive,
						    ARCHIVE_ERRNO_MISC,
						    "Path contains '..'");
						return (ARCHIVE_FAILED);
					}
				}
			}
		}
		if (separator)
			*dest++ = '/';
		while (*src != '\0' && *src != '/') {
			*dest++ = *src++;
		}
		if (*src == '\0')
			break;
		separator = *src++;
	}
	if (dest == a->name) {
		if (separator)
			*dest++ = '/';
		else
			*dest++ = '.';
	}
	*dest = '\0';
	return (ARCHIVE_OK);
}