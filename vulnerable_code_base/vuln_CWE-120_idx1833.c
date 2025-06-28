static void TreeTest(Jsi_Interp* interp) {
    Jsi_Tree *st, *wt, *mt;
    Jsi_TreeEntry *hPtr, *hPtr2;
    bool isNew, i;
    Jsi_TreeSearch srch;
    struct tdata {
        int n;
        int m;
    } t1, t2;
    char nbuf[100];
    wt = Jsi_TreeNew(interp, JSI_KEYS_ONEWORD, NULL);
    mt = Jsi_TreeNew(interp, sizeof(struct tdata), NULL);
    Jsi_TreeSet(wt, wt,(void*)0x88);
    Jsi_TreeSet(wt, mt,(void*)0x99);
    printf("WT: %p\n", Jsi_TreeGet(wt, mt));
    printf("WT2: %p\n", Jsi_TreeGet(wt, wt));
    Jsi_TreeDelete(wt);
    t1.n = 0; t1.m = 1;
    t2.n = 1; t2.m = 2;
    Jsi_TreeSet(mt, &t1,(void*)0x88);
    Jsi_TreeSet(mt, &t2,(void*)0x99);
    Jsi_TreeSet(mt, &t2,(void*)0x98);
    printf("CT: %p\n", Jsi_TreeGet(mt, &t1));
    printf("CT2: %p\n", Jsi_TreeGet(mt, &t2));
    Jsi_TreeDelete(mt);
    st = Jsi_TreeNew(interp, JSI_KEYS_STRING, NULL);
    hPtr = Jsi_TreeEntryNew(st, "bob", &isNew);
    Jsi_TreeValueSet(hPtr, (void*)99);
    Jsi_TreeSet(st, "zoe",(void*)77);
    hPtr2 = Jsi_TreeSet(st, "ted",(void*)55);
    Jsi_TreeSet(st, "philip",(void*)66);
    Jsi_TreeSet(st, "alice",(void*)77);
    puts("SRCH");
    for (hPtr=Jsi_TreeSearchFirst(st,&srch,  JSI_TREE_ORDER_IN, NULL); hPtr; hPtr=Jsi_TreeSearchNext(&srch))
        mycall(st, hPtr, NULL);
    Jsi_TreeSearchDone(&srch);
    puts("IN");
    Jsi_TreeWalk(st, mycall, NULL, JSI_TREE_ORDER_IN);
    puts("PRE");
    Jsi_TreeWalk(st, mycall, NULL, JSI_TREE_ORDER_PRE);
    puts("POST");
    Jsi_TreeWalk(st, mycall, NULL, JSI_TREE_ORDER_POST);
    puts("LEVEL");
    Jsi_TreeWalk(st, mycall, NULL, JSI_TREE_ORDER_LEVEL);
    Jsi_TreeEntryDelete(hPtr2);
    puts("INDEL");
    Jsi_TreeWalk(st, mycall, NULL, 0);
    for (i=0; i<1000; i++) {
        snprintf(nbuf, sizeof(nbuf), "name%d", i);
        Jsi_TreeSet(st, nbuf,(void*)i);
    }
    Jsi_TreeWalk(st, mycall, NULL, 0);
    for (i=0; i<1000; i++) {
        Jsi_TreeEntryDelete(st->root);
    }
    puts("OK");
    Jsi_TreeWalk(st, mycall, NULL, 0);
    Jsi_TreeDelete(st);
}