DefragDoSturgesNovakTest(int policy, u_char *expected, size_t expected_len)
{
    int i;
    int ret = 0;
    DefragInit();
    int id = 1;
    Packet *packets[17];
    memset(packets, 0x00, sizeof(packets));
    packets[0] = BuildTestPacket(id, 0, 1, 'A', 24);
    packets[1] = BuildTestPacket(id, 32 >> 3, 1, 'B', 16);
    packets[2] = BuildTestPacket(id, 48 >> 3, 1, 'C', 24);
    packets[3] = BuildTestPacket(id, 80 >> 3, 1, 'D', 8);
    packets[4] = BuildTestPacket(id, 104 >> 3, 1, 'E', 16);
    packets[5] = BuildTestPacket(id, 120 >> 3, 1, 'F', 24);
    packets[6] = BuildTestPacket(id, 144 >> 3, 1, 'G', 16);
    packets[7] = BuildTestPacket(id, 160 >> 3, 1, 'H', 16);
    packets[8] = BuildTestPacket(id, 176 >> 3, 1, 'I', 8);
    packets[9] = BuildTestPacket(id, 8 >> 3, 1, 'J', 32);
    packets[10] = BuildTestPacket(id, 48 >> 3, 1, 'K', 24);
    packets[11] = BuildTestPacket(id, 72 >> 3, 1, 'L', 24);
    packets[12] = BuildTestPacket(id, 96 >> 3, 1, 'M', 24);
    packets[13] = BuildTestPacket(id, 128 >> 3, 1, 'N', 8);
    packets[14] = BuildTestPacket(id, 152 >> 3, 1, 'O', 8);
    packets[15] = BuildTestPacket(id, 160 >> 3, 1, 'P', 8);
    packets[16] = BuildTestPacket(id, 176 >> 3, 0, 'Q', 16);
    default_policy = policy;
    for (i = 0; i < 9; i++) {
        Packet *tp = Defrag(NULL, NULL, packets[i], NULL);
        if (tp != NULL) {
            SCFree(tp);
            goto end;
        }
        if (ENGINE_ISSET_EVENT(packets[i], IPV4_FRAG_OVERLAP)) {
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
        if (ENGINE_ISSET_EVENT(packets[i], IPV4_FRAG_OVERLAP)) {
            overlap++;
        }
    }
    if (!overlap) {
        goto end;
    }
    Packet *reassembled = Defrag(NULL, NULL, packets[16], NULL);
    if (reassembled == NULL) {
        goto end;
    }
    if (IPV4_GET_HLEN(reassembled) != 20) {
        goto end;
    }
    if (IPV4_GET_IPLEN(reassembled) != 20 + 192) {
        goto end;
    }
    if (memcmp(GET_PKT_DATA(reassembled) + 20, expected, expected_len) != 0) {
        goto end;
    }
    SCFree(reassembled);
    if (defrag_context->frag_pool->outstanding != 0) {
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