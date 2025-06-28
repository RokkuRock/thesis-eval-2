IPV6DefragDoSturgesNovakTest(int policy, u_char *expected, size_t expected_len)
{
    int i;
    int ret = 0;
    DefragInit();
    int id = 1;
    Packet *packets[17];
    memset(packets, 0x00, sizeof(packets));
    packets[0] = IPV6BuildTestPacket(id, 0, 1, 'A', 24);
    packets[1] = IPV6BuildTestPacket(id, 32 >> 3, 1, 'B', 16);
    packets[2] = IPV6BuildTestPacket(id, 48 >> 3, 1, 'C', 24);
    packets[3] = IPV6BuildTestPacket(id, 80 >> 3, 1, 'D', 8);
    packets[4] = IPV6BuildTestPacket(id, 104 >> 3, 1, 'E', 16);
    packets[5] = IPV6BuildTestPacket(id, 120 >> 3, 1, 'F', 24);
    packets[6] = IPV6BuildTestPacket(id, 144 >> 3, 1, 'G', 16);
    packets[7] = IPV6BuildTestPacket(id, 160 >> 3, 1, 'H', 16);
    packets[8] = IPV6BuildTestPacket(id, 176 >> 3, 1, 'I', 8);
    packets[9] = IPV6BuildTestPacket(id, 8 >> 3, 1, 'J', 32);
    packets[10] = IPV6BuildTestPacket(id, 48 >> 3, 1, 'K', 24);
    packets[11] = IPV6BuildTestPacket(id, 72 >> 3, 1, 'L', 24);
    packets[12] = IPV6BuildTestPacket(id, 96 >> 3, 1, 'M', 24);
    packets[13] = IPV6BuildTestPacket(id, 128 >> 3, 1, 'N', 8);
    packets[14] = IPV6BuildTestPacket(id, 152 >> 3, 1, 'O', 8);
    packets[15] = IPV6BuildTestPacket(id, 160 >> 3, 1, 'P', 8);
    packets[16] = IPV6BuildTestPacket(id, 176 >> 3, 0, 'Q', 16);
    default_policy = policy;
    for (i = 0; i < 9; i++) {
        Packet *tp = Defrag(NULL, NULL, packets[i], NULL);
        if (tp != NULL) {
            SCFree(tp);
            goto end;
        }
        if (ENGINE_ISSET_EVENT(packets[i], IPV6_FRAG_OVERLAP)) {
            goto end;
        }
    }
    int overlap = 0;
    for (; i < 16; i++) {
        Packet *tp = Defrag(NULL, NULL, packets[i], NULL);
        if (tp != NULL) {
            SCFree(tp);
            goto end;
        }
        if (ENGINE_ISSET_EVENT(packets[i], IPV6_FRAG_OVERLAP)) {
            overlap++;
        }
    }
    if (!overlap)
        goto end;
    Packet *reassembled = Defrag(NULL, NULL, packets[16], NULL);
    if (reassembled == NULL)
        goto end;
    if (memcmp(GET_PKT_DATA(reassembled) + 40, expected, expected_len) != 0)
        goto end;
    if (IPV6_GET_PLEN(reassembled) != 192)
        goto end;
    SCFree(reassembled);
    if (defrag_context->frag_pool->outstanding != 0) {
        printf("defrag_context->frag_pool->outstanding %u: ", defrag_context->frag_pool->outstanding);
        goto end;
    }
    ret = 1;
end:
    for (i = 0; i < 17; i++) {
        SCFree(packets[i]);
    }
    DefragDestroy();
    return ret;
}