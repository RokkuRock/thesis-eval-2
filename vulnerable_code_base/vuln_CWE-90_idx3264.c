static int get_filter(struct sss_certmap_ctx *ctx,
                      struct ldap_mapping_rule *parsed_mapping_rule,
                      struct sss_cert_content *cert_content,
                      char **filter)
{
    struct ldap_mapping_rule_comp *comp;
    char *result = NULL;
    char *expanded = NULL;
    int ret;
    result = talloc_strdup(ctx, "");
    if (result == NULL) {
        return ENOMEM;
    }
    for (comp = parsed_mapping_rule->list; comp != NULL; comp = comp->next) {
        if (comp->type == comp_string) {
            result = talloc_strdup_append(result, comp->val);
        } else if (comp->type == comp_template) {
            ret = expand_template(ctx, comp->parsed_template, cert_content,
                                  &expanded);
            if (ret != 0) {
                CM_DEBUG(ctx, "Failed to expanded template.");
                goto done;
            }
            result = talloc_strdup_append(result, expanded);
            talloc_free(expanded);
            expanded = NULL;
            if (result == NULL) {
                ret = ENOMEM;
                goto done;
            }
        } else {
            ret = EINVAL;
            CM_DEBUG(ctx, "Unsupported component type.");
            goto done;
        }
    }
    ret = 0;
done:
    talloc_free(expanded);
    if (ret == 0) {
        *filter = result;
    } else {
        talloc_free(result);
    }
    return ret;
}