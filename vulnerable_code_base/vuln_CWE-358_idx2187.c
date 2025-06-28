DefragTimeoutTest(void)
{
    int i;
    int ret = 0;
    if (ConfSet("defrag.trackers", "16") != 1) {
        printf("ConfSet failed: ");
        goto end;
    }
    DefragInit();
    for (i = 0; i < 16; i++) {
        Packet *p = BuildTestPacket(i, 0, 1, 'A' + i, 16);
        if (p == NULL)
            goto end;
        Packet *tp = Defrag(NULL, NULL, p, NULL);
        SCFree(p);
        if (tp != NULL) {
            SCFree(tp);
            goto end;
        }
    }
    Packet *p = BuildTestPacket(99, 0, 1, 'A' + i, 16);
    if (p == NULL)
        goto end;
    p->ts.tv_sec += (defrag_context->timeout + 1);
    Packet *tp = Defrag(NULL, NULL, p, NULL);
    if (tp != NULL) {
        SCFree(tp);
        goto end;
    }
    DefragTracker *tracker = DefragLookupTrackerFromHash(p);
    if (tracker == NULL)
        goto end;
    if (tracker->id != 99)
        goto end;
    SCFree(p);
    ret = 1;
end:
    DefragDestroy();
    return ret;
}