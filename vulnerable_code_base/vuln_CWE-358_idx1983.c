static int DefragTrackerReuseTest(void)
{
    int ret = 0;
    int id = 1;
    Packet *p1 = NULL;
    DefragTracker *tracker1 = NULL, *tracker2 = NULL;
    DefragInit();
    p1 = BuildTestPacket(id, 0, 0, 'A', 8);
    if (p1 == NULL) {
        goto end;
    }
    tracker1 = DefragGetTracker(NULL, NULL, p1);
    if (tracker1 == NULL) {
        goto end;
    }
    if (tracker1->seen_last) {
        goto end;
    }
    if (tracker1->remove) {
        goto end;
    }
    DefragTrackerRelease(tracker1);
    tracker2 = DefragGetTracker(NULL, NULL, p1);
    if (tracker2 == NULL) {
        goto end;
    }
    if (tracker2 != tracker1) {
        goto end;
    }
    DefragTrackerRelease(tracker1);
    tracker1->remove = 1;
    tracker2 = DefragGetTracker(NULL, NULL, p1);
    if (tracker2 == NULL) {
        goto end;
    }
    if (tracker2 == tracker1) {
        goto end;
    }
    if (tracker2->remove) {
        goto end;
    }
    ret = 1;
end:
    if (p1 != NULL) {
        SCFree(p1);
    }
    DefragDestroy();
    return ret;
}