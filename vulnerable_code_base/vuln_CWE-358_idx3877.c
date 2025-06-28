DefragIPv4TooLargeTest(void)
{
    DefragContext *dc = NULL;
    Packet *p = NULL;
    int ret = 0;
    DefragInit();
    dc = DefragContextNew();
    if (dc == NULL)
        goto end;
    p = BuildTestPacket(1, 8183, 0, 'A', 71);
    if (p == NULL)
        goto end;
    if (Defrag(NULL, NULL, p, NULL) != NULL)
        goto end;
    if (!ENGINE_ISSET_EVENT(p, IPV4_FRAG_PKT_TOO_LARGE))
        goto end;
    if (dc->frag_pool->outstanding != 0)
        return 0;
    ret = 1;
end:
    if (dc != NULL)
        DefragContextDestroy(dc);
    if (p != NULL)
        SCFree(p);
    DefragDestroy();
    return ret;
}