void addReply(redisClient *c, robj *obj) {
    if (_installWriteEvent(c) != REDIS_OK) return;
    redisAssert(!server.vm_enabled || obj->storage == REDIS_VM_MEMORY);
    if (obj->encoding == REDIS_ENCODING_RAW) {
        if (_addReplyToBuffer(c,obj->ptr,sdslen(obj->ptr)) != REDIS_OK)
            _addReplyObjectToList(c,obj);
    } else {
        obj = getDecodedObject(obj);
        if (_addReplyToBuffer(c,obj->ptr,sdslen(obj->ptr)) != REDIS_OK)
            _addReplyObjectToList(c,obj);
        decrRefCount(obj);
    }
}