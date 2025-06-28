decode_sequence(const uint8_t *asn1, size_t len, const struct seq_info *seq,
                void *val)
{
    krb5_error_code ret;
    const uint8_t *contents;
    size_t i, j, clen;
    taginfo t;
    assert(seq->n_fields > 0);
    for (i = 0; i < seq->n_fields; i++) {
        if (len == 0)
            break;
        ret = get_tag(asn1, len, &t, &contents, &clen, &asn1, &len);
        if (ret)
            goto error;
        for (; i < seq->n_fields; i++) {
            if (check_atype_tag(seq->fields[i], &t))
                break;
            ret = omit_atype(seq->fields[i], val);
            if (ret)
                goto error;
        }
        if (i == seq->n_fields)
            break;
        ret = decode_atype(&t, contents, clen, seq->fields[i], val);
        if (ret)
            goto error;
    }
    for (; i < seq->n_fields; i++) {
        ret = omit_atype(seq->fields[i], val);
        if (ret)
            goto error;
    }
    return 0;
error:
    for (j = 0; j < i; j++)
        free_atype(seq->fields[j], val);
    for (j = 0; j < i; j++)
        free_atype_ptr(seq->fields[j], val);
    return ret;
}