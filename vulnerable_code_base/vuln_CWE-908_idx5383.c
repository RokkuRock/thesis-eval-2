void ZydisFormatterBufferInit(ZydisFormatterBuffer* buffer, char* user_buffer,
    ZyanUSize length)
{
    ZYAN_ASSERT(buffer);
    ZYAN_ASSERT(user_buffer);
    ZYAN_ASSERT(length);
    buffer->is_token_list              = ZYAN_FALSE;
    buffer->string.flags               = ZYAN_STRING_HAS_FIXED_CAPACITY;
    buffer->string.vector.allocator    = ZYAN_NULL;
    buffer->string.vector.element_size = sizeof(char);
    buffer->string.vector.size         = 1;
    buffer->string.vector.capacity     = length;
    buffer->string.vector.data         = user_buffer;
    *user_buffer = '\0';
}