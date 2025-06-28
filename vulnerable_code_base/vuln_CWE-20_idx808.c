int prepareForShutdown() {
    redisLog(REDIS_WARNING,"User requested shutdown, saving DB...");
    if (server.bgsavechildpid != -1) {
        redisLog(REDIS_WARNING,"There is a live saving child. Killing it!");
        kill(server.bgsavechildpid,SIGKILL);
        rdbRemoveTempFile(server.bgsavechildpid);
    }
    if (server.appendonly) {
        aof_fsync(server.appendfd);
        if (server.vm_enabled) unlink(server.vm_swap_file);
    } else if (server.saveparamslen > 0) {
        if (rdbSave(server.dbfilename) != REDIS_OK) {
            redisLog(REDIS_WARNING,"Error trying to save the DB, can't exit");
            return REDIS_ERR;
        }
    } else {
        redisLog(REDIS_WARNING,"Not saving DB.");
    }
    if (server.daemonize) unlink(server.pidfile);
    redisLog(REDIS_WARNING,"Server exit now, bye bye...");
    return REDIS_OK;
}