DefragIPv4NoDataTest(void)
{
    DefragContext *dc = NULL;
    Packet *p = NULL;
    int id = 12;
    int ret = 0;
    DefragInit();
    dc = DefragContextNew();
    if (dc == NULL)
        goto end;
    p = BuildTestPacket(id, 1, 0, 'A', 0);
    if (p == NULL)
        goto end;
    if (Defrag(NULL, NULL, p, NULL) != NULL)
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