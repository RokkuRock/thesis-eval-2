void freeClient(redisClient *c) {
    listNode *ln;
    sdsfree(c->querybuf);
    c->querybuf = NULL;
    if (c->flags & REDIS_BLOCKED)
        unblockClientWaitingData(c);
    unwatchAllKeys(c);
    listRelease(c->watched_keys);
    pubsubUnsubscribeAllChannels(c,0);
    pubsubUnsubscribeAllPatterns(c,0);
    dictRelease(c->pubsub_channels);
    listRelease(c->pubsub_patterns);
    aeDeleteFileEvent(server.el,c->fd,AE_READABLE);
    aeDeleteFileEvent(server.el,c->fd,AE_WRITABLE);
    listRelease(c->reply);
    freeClientArgv(c);
    close(c->fd);
    ln = listSearchKey(server.clients,c);
    redisAssert(ln != NULL);
    listDelNode(server.clients,ln);
    if (c->flags & REDIS_IO_WAIT) {
        redisAssert(server.vm_enabled);
        if (listLength(c->io_keys) == 0) {
            ln = listSearchKey(server.io_ready_clients,c);
            redisAssert(ln != NULL);
            listDelNode(server.io_ready_clients,ln);
        } else {
            while (listLength(c->io_keys)) {
                ln = listFirst(c->io_keys);
                dontWaitForSwappedKey(c,ln->value);
            }
        }
        server.vm_blocked_clients--;
    }
    listRelease(c->io_keys);
    if (c->flags & REDIS_SLAVE) {
        if (c->replstate == REDIS_REPL_SEND_BULK && c->repldbfd != -1)
            close(c->repldbfd);
        list *l = (c->flags & REDIS_MONITOR) ? server.monitors : server.slaves;
        ln = listSearchKey(l,c);
        redisAssert(ln != NULL);
        listDelNode(l,ln);
    }
    if (c->flags & REDIS_MASTER) {
        server.master = NULL;
        server.replstate = REDIS_REPL_CONNECT;
        while (listLength(server.slaves)) {
            ln = listFirst(server.slaves);
            freeClient((redisClient*)ln->value);
        }
    }
    zfree(c->argv);
    freeClientMultiState(c);
    zfree(c);
}