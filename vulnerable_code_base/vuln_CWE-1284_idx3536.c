int encrypt_stream(FILE *infp, FILE *outfp, unsigned char* passwd, int passlen)
{
    aes_context aes_ctx;
    sha256_context sha_ctx;
    aescrypt_hdr aeshdr;
    sha256_t digest;
    unsigned char IV[16];
    unsigned char iv_key[48];
    unsigned i, j;
    size_t bytes_read;
    unsigned char buffer[32];
    unsigned char ipad[64], opad[64];
    time_t current_time;
    pid_t process_id;
    void *aesrand;
    unsigned char tag_buffer[256];
    if ((aesrand = aesrandom_open()) == NULL)
    {
        perror("Error open random:");
        return -1;
    }
    memset(iv_key, 0, 48);
    for (i=0; i<48; i+=16)
    {
        memset(buffer, 0, 32);
        sha256_starts(&sha_ctx);
        for(j=0; j<256; j++)
        {
            if ((bytes_read = aesrandom_read(aesrand, buffer, 32)) != 32)
            {
                fprintf(stderr, "Error: Couldn't read from random : %u\n",
                        (unsigned) bytes_read);
                aesrandom_close(aesrand);
                return -1;
            }
            sha256_update(&sha_ctx, buffer, 32);
        }
        sha256_finish(&sha_ctx, digest);
        memcpy(iv_key+i, digest, 16);
    }
    buffer[0] = 'A';
    buffer[1] = 'E';
    buffer[2] = 'S';
    buffer[3] = (unsigned char) 0x02;    
    buffer[4] = '\0';                    
    if (fwrite(buffer, 1, 5, outfp) != 5)
    {
        fprintf(stderr, "Error: Could not write out header data\n");
        aesrandom_close(aesrand);
        return -1;
    }
    j = 11 +                       
        strlen(PACKAGE_NAME) +     
        1 +                        
        strlen(PACKAGE_VERSION);   
    if (j < 256)
    {
        buffer[0] = '\0';
        buffer[1] = (unsigned char) (j & 0xff);
        if (fwrite(buffer, 1, 2, outfp) != 2)
        {
            fprintf(stderr, "Error: Could not write tag to AES file (1)\n");
            aesrandom_close(aesrand);
            return -1;
        }
        strncpy((char *)tag_buffer, "CREATED_BY", 255);
        tag_buffer[255] = '\0';
        if (fwrite(tag_buffer, 1, 11, outfp) != 11)
        {
            fprintf(stderr, "Error: Could not write tag to AES file (2)\n");
            aesrandom_close(aesrand);
            return -1;
        }
        sprintf((char *)tag_buffer, "%s %s", PACKAGE_NAME, PACKAGE_VERSION);
        j = strlen((char *)tag_buffer);
        if (fwrite(tag_buffer, 1, j, outfp) != j)
        {
            fprintf(stderr, "Error: Could not write tag to AES file (3)\n");
            aesrandom_close(aesrand);
            return -1;
        }
    }
    buffer[0] = '\0';
    buffer[1] = (unsigned char) 128;
    if (fwrite(buffer, 1, 2, outfp) != 2)
    {
        fprintf(stderr, "Error: Could not write tag to AES file (4)\n");
        aesrandom_close(aesrand);
        return -1;
    }
    memset(tag_buffer, 0, 128);
    if (fwrite(tag_buffer, 1, 128, outfp) != 128)
    {
        fprintf(stderr, "Error: Could not write tag to AES file (5)\n");
        aesrandom_close(aesrand);
        return -1;
    }
    buffer[0] = '\0';
    buffer[1] = '\0';
    if (fwrite(buffer, 1, 2, outfp) != 2)
    {
        fprintf(stderr, "Error: Could not write tag to AES file (6)\n");
        aesrandom_close(aesrand);
        return -1;
    }
    sha256_starts(  &sha_ctx);
    current_time = time(NULL);
    sha256_update(  &sha_ctx, (unsigned char *)&time, sizeof(current_time));
    process_id = getpid();
    sha256_update(  &sha_ctx, (unsigned char *)&process_id, sizeof(process_id));
    for (i=0; i<256; i++)
    {
        if (aesrandom_read(aesrand, buffer, 32) != 32)
        {
            fprintf(stderr, "Error: Couldn't read from /dev/random\n");
            aesrandom_close(aesrand);
            return -1;
        }
        sha256_update(  &sha_ctx,
                        buffer,
                        32);
    }
    sha256_finish(  &sha_ctx, digest);
    memcpy(IV, digest, 16);
    aesrandom_close(aesrand);
    if (fwrite(IV, 1, 16, outfp) != 16)
    {
        fprintf(stderr, "Error: Could not write out initialization vector\n");
        return -1;
    }
    memset(digest, 0, 32);
    memcpy(digest, IV, 16);
    for(i=0; i<8192; i++)
    {
        sha256_starts(  &sha_ctx);
        sha256_update(  &sha_ctx, digest, 32);
        sha256_update(  &sha_ctx,
                        passwd,
                        (unsigned long)passlen);
        sha256_finish(  &sha_ctx,
                        digest);
    }
    aes_set_key(&aes_ctx, digest, 256);
    memset(ipad, 0x36, 64);
    memset(opad, 0x5C, 64);
    for(i=0; i<32; i++)
    {
        ipad[i] ^= digest[i];
        opad[i] ^= digest[i];
    }
    sha256_starts(&sha_ctx);
    sha256_update(&sha_ctx, ipad, 64);
    for(i=0; i<48; i+=16)
    {
        memcpy(buffer, iv_key+i, 16);
        for(j=0; j<16; j++)
        {
            buffer[j] ^= IV[j];
        }
        aes_encrypt(&aes_ctx, buffer, buffer);
        sha256_update(&sha_ctx, buffer, 16);
        if (fwrite(buffer, 1, 16, outfp) != 16)
        {
            fprintf(stderr, "Error: Could not write iv_key data\n");
            return -1;
        }
        memcpy(IV, buffer, 16);
    }
    sha256_finish(&sha_ctx, digest);
    sha256_starts(&sha_ctx);
    sha256_update(&sha_ctx, opad, 64);
    sha256_update(&sha_ctx, digest, 32);
    sha256_finish(&sha_ctx, digest);
    if (fwrite(digest, 1, 32, outfp) != 32)
    {
        fprintf(stderr, "Error: Could not write iv_key HMAC\n");
        return -1;
    }
    memcpy(IV, iv_key, 16);
    aes_set_key(&aes_ctx, iv_key+16, 256);
    memset(ipad, 0x36, 64);
    memset(opad, 0x5C, 64);
    for(i=0; i<32; i++)
    {
        ipad[i] ^= iv_key[i+16];
        opad[i] ^= iv_key[i+16];
    }
    memset_secure(iv_key, 0, 48);
    sha256_starts(&sha_ctx);
    sha256_update(&sha_ctx, ipad, 64);
    aeshdr.last_block_size = 0;
    while ((bytes_read = fread(buffer, 1, 16, infp)) > 0)
    {
        for(i=0; i<16; i++)
        {
            buffer[i] ^= IV[i];
        }
        aes_encrypt(&aes_ctx, buffer, buffer);
        sha256_update(&sha_ctx, buffer, 16);
        if (fwrite(buffer, 1, 16, outfp) != 16)
        {
            fprintf(stderr, "Error: Could not write to output file\n");
            return -1;
        }
        memcpy(IV, buffer, 16);
        aeshdr.last_block_size = bytes_read;
    }
    if (ferror(infp))
    {
        fprintf(stderr, "Error: Couldn't read input file\n");
        return -1;
    }
    buffer[0] = (char) (aeshdr.last_block_size & 0x0F);
    if (fwrite(buffer, 1, 1, outfp) != 1)
    {
        fprintf(stderr, "Error: Could not write the file size modulo\n");
        return -1;
    }
    sha256_finish(&sha_ctx, digest);
    sha256_starts(&sha_ctx);
    sha256_update(&sha_ctx, opad, 64);
    sha256_update(&sha_ctx, digest, 32);
    sha256_finish(&sha_ctx, digest);
    if (fwrite(digest, 1, 32, outfp) != 32)
    {
        fprintf(stderr, "Error: Could not write the file HMAC\n");
        return -1;
    }
    if (fflush(outfp))
    {
        fprintf(stderr, "Error: Could not flush output file buffer\n");
        return -1;
    }
    return 0;
}