int ntlm_decode_target_info(struct ntlm_ctx *ctx, struct ntlm_buffer *buffer,
                            char **nb_computer_name, char **nb_domain_name,
                            char **dns_computer_name, char **dns_domain_name,
                            char **dns_tree_name, char **av_target_name,
                            uint32_t *av_flags, uint64_t *av_timestamp,
                            struct ntlm_buffer *av_single_host,
                            struct ntlm_buffer *av_cb)
{
    struct wire_av_pair *av_pair;
    uint16_t av_id = (uint16_t)-1;
    uint16_t av_len = (uint16_t)-1;
    struct ntlm_buffer sh = { NULL, 0 };
    struct ntlm_buffer cb = { NULL, 0 };
    char *nb_computer = NULL;
    char *nb_domain = NULL;
    char *dns_computer = NULL;
    char *dns_domain = NULL;
    char *dns_tree = NULL;
    char *av_target = NULL;
    size_t data_offs = 0;
    uint64_t timestamp = 0;
    uint32_t flags = 0;
    int ret = 0;
    while (data_offs + 4 <= buffer->length) {
        av_pair = (struct wire_av_pair *)&buffer->data[data_offs];
        data_offs += 4;
        av_id = le16toh(av_pair->av_id);
        av_len = le16toh(av_pair->av_len);
        if (av_len > buffer->length - data_offs) {
            ret = ERR_DECODE;
            goto done;
        }
        data_offs += av_len;
        switch (av_id) {
        case MSV_AV_CHANNEL_BINDINGS:
            if (!av_cb) continue;
            cb.data = av_pair->value;
            cb.length = av_len;
            break;
        case MSV_AV_TARGET_NAME:
            if (!av_target_name) continue;
            ret = ntlm_decode_av_pair_u16l_str(ctx, av_pair, &av_target);
            if (ret) goto done;
            break;
        case MSV_AV_SINGLE_HOST:
            if (!av_single_host) continue;
            sh.data = av_pair->value;
            sh.length = av_len;
            break;
        case MSV_AV_TIMESTAMP:
            if (!av_timestamp) continue;
            memcpy(&timestamp, av_pair->value, sizeof(timestamp));
            timestamp = le64toh(timestamp);
            break;
        case MSV_AV_FLAGS:
            if (!av_flags) continue;
            memcpy(&flags, av_pair->value, sizeof(flags));
            flags = le32toh(flags);
            break;
        case MSV_AV_DNS_TREE_NAME:
            if (!dns_tree_name) continue;
            ret = ntlm_decode_av_pair_u16l_str(ctx, av_pair, &dns_tree);
            if (ret) goto done;
            break;
        case MSV_AV_DNS_DOMAIN_NAME:
            if (!dns_domain_name) continue;
            ret = ntlm_decode_av_pair_u16l_str(ctx, av_pair, &dns_domain);
            if (ret) goto done;
            break;
        case MSV_AV_DNS_COMPUTER_NAME:
            if (!dns_computer_name) continue;
            ret = ntlm_decode_av_pair_u16l_str(ctx, av_pair, &dns_computer);
            if (ret) goto done;
            break;
        case MSV_AV_NB_DOMAIN_NAME:
            if (!nb_domain_name) continue;
            ret = ntlm_decode_av_pair_u16l_str(ctx, av_pair, &nb_domain);
            if (ret) goto done;
            break;
        case MSV_AV_NB_COMPUTER_NAME:
            if (!nb_computer_name) continue;
            ret = ntlm_decode_av_pair_u16l_str(ctx, av_pair, &nb_computer);
            if (ret) goto done;
            break;
        default:
            break;
        }
        if (av_id == MSV_AV_EOL) break;
    }
    if (av_id != MSV_AV_EOL || av_len != 0) {
        ret = ERR_DECODE;
    }
done:
    if (ret) {
        ntlm_free_buffer_data(&sh);
        ntlm_free_buffer_data(&cb);
        safefree(nb_computer);
        safefree(nb_domain);
        safefree(dns_computer);
        safefree(dns_domain);
        safefree(dns_tree);
        safefree(av_target);
    } else {
        if (nb_computer_name) *nb_computer_name = nb_computer;
        if (nb_domain_name) *nb_domain_name = nb_domain;
        if (dns_computer_name) *dns_computer_name = dns_computer;
        if (dns_domain_name) *dns_domain_name = dns_domain;
        if (dns_tree_name) *dns_tree_name = dns_tree;
        if (av_target_name) *av_target_name = av_target;
        if (av_timestamp) *av_timestamp = timestamp;
        if (av_single_host) *av_single_host = sh;
        if (av_flags) *av_flags = flags;
        if (av_cb) *av_cb = cb;
    }
    return ret;
}