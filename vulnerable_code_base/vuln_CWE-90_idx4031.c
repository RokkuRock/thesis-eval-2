int sss_certmap_get_search_filter(struct sss_certmap_ctx *ctx,
                                  const uint8_t *der_cert, size_t der_size,
                                  char **_filter, char ***_domains)
{
    int ret;
    struct match_map_rule *r;
    struct priority_list *p;
    struct sss_cert_content *cert_content = NULL;
    char *filter = NULL;
    char **domains = NULL;
    size_t c;
    if (_filter == NULL || _domains == NULL) {
        return EINVAL;
    }
    ret = sss_cert_get_content(ctx, der_cert, der_size, &cert_content);
    if (ret != 0) {
        CM_DEBUG(ctx, "Failed to get certificate content [%d].", ret);
        return ret;
    }
    if (ctx->prio_list == NULL) {
        if (ctx->default_mapping_rule == NULL) {
            CM_DEBUG(ctx, "No matching or mapping rules available.");
            return EINVAL;
        }
        ret = get_filter(ctx, ctx->default_mapping_rule, cert_content, &filter);
        goto done;
    }
    for (p = ctx->prio_list; p != NULL; p = p->next) {
        for (r = p->rule_list; r != NULL; r = r->next) {
            ret = do_match(ctx, r->parsed_match_rule, cert_content);
            if (ret == 0) {
                ret = get_filter(ctx, r->parsed_mapping_rule, cert_content,
                                 &filter);
                if (ret != 0) {
                    CM_DEBUG(ctx, "Failed to get filter");
                    goto done;
                }
                if (r->domains != NULL) {
                    for (c = 0; r->domains[c] != NULL; c++);
                    domains = talloc_zero_array(ctx, char *, c + 1);
                    if (domains == NULL) {
                        ret = ENOMEM;
                        goto done;
                    }
                    for (c = 0; r->domains[c] != NULL; c++) {
                        domains[c] = talloc_strdup(domains, r->domains[c]);
                        if (domains[c] == NULL) {
                            ret = ENOMEM;
                            goto done;
                        }
                    }
                }
                ret = 0;
                goto done;
            }
        }
    }
    ret = ENOENT;
done:
    talloc_free(cert_content);
    if (ret == 0) {
        *_filter = filter;
        *_domains = domains;
    } else {
        talloc_free(filter);
        talloc_free(domains);
    }
    return ret;
}