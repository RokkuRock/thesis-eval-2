int ipfilter(struct pico_frame *f)
{
    struct filter_node temp;
    struct pico_ipv4_hdr *ipv4_hdr = (struct pico_ipv4_hdr *) f->net_hdr;
    struct pico_trans *trans;
    struct pico_icmp4_hdr *icmp_hdr;
    memset(&temp, 0u, sizeof(struct filter_node));
    temp.fdev = f->dev;
    temp.out_addr = ipv4_hdr->dst.addr;
    temp.in_addr = ipv4_hdr->src.addr;
    if ((ipv4_hdr->proto == PICO_PROTO_TCP) || (ipv4_hdr->proto == PICO_PROTO_UDP)) {
        trans = (struct pico_trans *) f->transport_hdr;
        temp.out_port = short_be(trans->dport);
        temp.in_port = short_be(trans->sport);
    }
    else if(ipv4_hdr->proto == PICO_PROTO_ICMP4) {
        icmp_hdr = (struct pico_icmp4_hdr *) f->transport_hdr;
        if(icmp_hdr->type == PICO_ICMP_UNREACH && icmp_hdr->code == PICO_ICMP_UNREACH_FILTER_PROHIB)
            return 0;
    }
    temp.proto = ipv4_hdr->proto;
    temp.priority = f->priority;
    temp.tos = ipv4_hdr->tos;
    return ipfilter_apply_filter(f, &temp);
}