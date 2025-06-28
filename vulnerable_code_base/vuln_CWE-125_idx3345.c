static u32 crc32sum(u32 crc, u8 * RESTRICT buf, size_t size) {
    while (size--) crc = crc32Table[(crc ^ *(buf++)) & 0xff] ^ (crc >> 8);
    return crc;
}