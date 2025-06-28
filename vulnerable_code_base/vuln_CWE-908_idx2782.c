void ZydisFormatterBufferInitTokenized(ZydisFormatterBuffer* buffer,
    ZydisFormatterToken** first_token, void* user_buffer, ZyanUSize length)
{
    ZYAN_ASSERT(buffer);
    ZYAN_ASSERT(first_token);
    ZYAN_ASSERT(user_buffer);
    ZYAN_ASSERT(length);
    *first_token = user_buffer;
    (*first_token)->type = ZYDIS_TOKEN_INVALID;
    (*first_token)->next = 0;
    user_buffer = (ZyanU8*)user_buffer + sizeof(ZydisFormatterToken);
    length -= sizeof(ZydisFormatterToken);
    buffer->is_token_list              = ZYAN_TRUE;
    buffer->capacity                   = length;
    buffer->string.flags               = ZYAN_STRING_HAS_FIXED_CAPACITY;
    buffer->string.vector.allocator    = ZYAN_NULL;
    buffer->string.vector.element_size = sizeof(char);
    buffer->string.vector.size         = 1;
    buffer->string.vector.capacity     = length;
    buffer->string.vector.data         = user_buffer;
    *(char*)user_buffer = '\0';
}