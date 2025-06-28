static uint32_t parse_user_name(uint32_t *minor_status,
                                const char *str, size_t len,
                                char **domain, char **username)
{
    uint32_t retmaj;
    uint32_t retmin;
    char *at, *sep;
    if (len > MAX_NAME_LEN) {
        return GSSERRS(ERR_NAMETOOLONG, GSS_S_BAD_NAME);
    }
    *username = NULL;
    *domain = NULL;
    at = memchr(str, '@', len);
    sep = memchr(str, '\\', len);
    if (at && sep) {
        char strbuf[len + 1];
        char *buf = strbuf;
        bool domain_handled = false;
        memcpy(buf, str, len);
        buf[len] = '\0';
        sep = buf + (sep - str);
        at = buf + (at - str);
        if (sep > at) {
            if (*(sep + 1) == '@') {
                set_GSSERR(EINVAL);
                goto done;
            }
        } else if (at - sep == 1) {
            sep = NULL;
        }
        if (sep) {
            domain_handled = true;
            *sep = '\0';
            *domain = strdup(buf);
            if (NULL == *domain) {
                set_GSSERR(ENOMEM);
                goto done;
            }
            len = len - (sep - buf) - 1;
            buf = sep + 1;
        }
        for (at = strchr(buf, '@'); at != NULL; at = strchr(at, '@')) {
            if (*(at - 1) == '\\') {
                if (domain_handled) {
                    free(*domain);
                    *domain = NULL;
                    set_GSSERR(EINVAL);
                    goto done;
                }
                memmove(at - 1, at, len - (at - buf) + 1);
            } else if (!domain_handled) {
                *at = '\0';
                *domain = strdup(at + 1);
                if (NULL == *domain) {
                    set_GSSERR(ENOMEM);
                    goto done;
                }
            }
            at += 1;
        }
        *username = strdup(buf);
        if (NULL == *username) {
            set_GSSERR(ENOMEM);
            goto done;
        }
        set_GSSERRS(0, GSS_S_COMPLETE);
        goto done;
    }
    if (sep) {
        retmaj = string_split(&retmin, '\\', str, len, domain, username);
        goto done;
    }
    if (at) {
        retmaj = string_split(&retmin, '@', str, len, username, domain);
        goto done;
    }
    *username = strndup(str, len);
    if (NULL == *username) {
        set_GSSERR(ENOMEM);
    }
    set_GSSERRS(0, GSS_S_COMPLETE);
done:
    return GSSERR();
}