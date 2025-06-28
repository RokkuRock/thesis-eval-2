static bool checkreturn pb_dec_bytes(pb_istream_t *stream, const pb_field_t *field, void *dest)
{
    uint32_t size;
    pb_bytes_array_t *bdest;
    if (!pb_decode_varint32(stream, &size))
        return false;
    if (PB_ATYPE(field->type) == PB_ATYPE_POINTER)
    {
#ifndef PB_ENABLE_MALLOC
        PB_RETURN_ERROR(stream, "no malloc support");
#else
        if (!allocate_field(stream, dest, PB_BYTES_ARRAY_T_ALLOCSIZE(size), 1))
            return false;
        bdest = *(pb_bytes_array_t**)dest;
#endif
    }
    else
    {
        if (PB_BYTES_ARRAY_T_ALLOCSIZE(size) > field->data_size)
            PB_RETURN_ERROR(stream, "bytes overflow");
        bdest = (pb_bytes_array_t*)dest;
    }
    if (size > PB_SIZE_MAX)
    {
        PB_RETURN_ERROR(stream, "bytes overflow");
    }
    bdest->size = (pb_size_t)size;
    return pb_read(stream, bdest->bytes, size);
}