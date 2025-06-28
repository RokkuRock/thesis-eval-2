void queue_push(register Queue *qp, size_t extra_length, char const *info)
{
    register char *cp;
    size_t memory_length;
    size_t available_length;
    size_t begin_length;
    size_t n_begin;
    size_t q_length;
    if (!extra_length)
        return;
    memory_length    = qp->d_memory_end - qp->d_memory;
    q_length = 
        qp->d_read <= qp->d_write ?
            (size_t)(qp->d_write - qp->d_read)
        :
            memory_length - (qp->d_read - qp->d_write);
    available_length = memory_length - q_length - 1;
    if (message_show(MSG_INFO))
        message("push_front %u bytes in `%s'", (unsigned)extra_length, info);
    if (extra_length > available_length)
    {
        memory_length += extra_length - available_length + BLOCK_QUEUE;
        cp = new_memory(memory_length, sizeof(char));
        if (message_show(MSG_INFO))
            message("Reallocating queue at %p to %p", qp->d_memory, cp);
        if (qp->d_read > qp->d_write)                
        {
            size_t tail_len = qp->d_memory_end - qp->d_read;
            memcpy(cp, qp->d_read, tail_len);        
            memcpy(cp + tail_len, qp->d_memory, 
                                    (size_t)(qp->d_write - qp->d_memory));
            qp->d_write = cp + q_length;
            qp->d_read = cp;
        }
        else                                         
        {
            memcpy(cp, qp->d_memory, memory_length); 
            qp->d_read = cp + (qp->d_read - qp->d_memory);
            qp->d_write = cp + (qp->d_write - qp->d_memory);
        }
        free(qp->d_memory);                          
        qp->d_memory_end = cp + memory_length;       
        qp->d_memory = cp;                           
    }
    begin_length = qp->d_read - qp->d_memory;    
    n_begin = extra_length <= begin_length ?     
                    extra_length                 
                :
                    begin_length;
    memcpy                                       
    (                                            
        qp->d_read -= n_begin,
        info + extra_length - n_begin,
        n_begin
    );
    if (extra_length > begin_length)             
    {
        extra_length -= begin_length;            
        memcpy                                   
        (                                        
            qp->d_read = qp->d_memory_end - extra_length,
            info,
            extra_length
        );
    }
}