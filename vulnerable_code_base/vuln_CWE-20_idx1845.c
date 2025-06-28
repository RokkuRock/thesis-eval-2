void beforeSleep(struct aeEventLoop *eventLoop) {
    REDIS_NOTUSED(eventLoop);
    listNode *ln;
    redisClient *c;
    if (server.vm_enabled && listLength(server.io_ready_clients)) {
        listIter li;
        listRewind(server.io_ready_clients,&li);
        while((ln = listNext(&li))) {
            c = ln->value;
            struct redisCommand *cmd;
            listDelNode(server.io_ready_clients,ln);
            c->flags &= (~REDIS_IO_WAIT);
            server.vm_blocked_clients--;
            aeCreateFileEvent(server.el, c->fd, AE_READABLE,
                readQueryFromClient, c);
            cmd = lookupCommand(c->argv[0]->ptr);
            redisAssert(cmd != NULL);
            call(c,cmd);
            resetClient(c);
            if (c->querybuf && sdslen(c->querybuf) > 0)
                processInputBuffer(c);
        }
    }
    while (listLength(server.unblocked_clients)) {
        ln = listFirst(server.unblocked_clients);
        redisAssert(ln != NULL);
        c = ln->value;
        listDelNode(server.unblocked_clients,ln);
        if (c->querybuf && sdslen(c->querybuf) > 0)
            processInputBuffer(c);
    }
    flushAppendOnlyFile();
}