static int blosc_c(struct thread_context* thread_context, int32_t bsize,
                   int32_t leftoverblock, int32_t ntbytes, int32_t maxbytes,
                   const uint8_t* src, const int32_t offset, uint8_t* dest,
                   uint8_t* tmp, uint8_t* tmp2) {
  blosc2_context* context = thread_context->parent_context;
  int dont_split = (context->header_flags & 0x10) >> 4;
  int dict_training = context->use_dict && context->dict_cdict == NULL;
  int32_t j, neblock, nstreams;
  int32_t cbytes;                    
  int32_t ctbytes = 0;               
  int64_t maxout;
  int32_t typesize = context->typesize;
  const char* compname;
  int accel;
  const uint8_t* _src;
  uint8_t *_tmp = tmp, *_tmp2 = tmp2;
  uint8_t *_tmp3 = thread_context->tmp4;
  int last_filter_index = last_filter(context->filters, 'c');
  bool memcpyed = context->header_flags & (uint8_t)BLOSC_MEMCPYED;
  if (last_filter_index >= 0 || context->prefilter != NULL) {
    if (memcpyed && context->prefilter != NULL) {
      _src = pipeline_c(thread_context, bsize, src, offset, dest, _tmp2, _tmp3);
      if (_src == NULL) {
        return -9;   
      }
      return bsize;
    }
    _src = pipeline_c(thread_context, bsize, src, offset, _tmp, _tmp2, _tmp3);
    if (_src == NULL) {
      return -9;   
    }
  } else {
    _src = src + offset;
  }
  assert(context->clevel > 0);
  accel = get_accel(context);
  if (!dont_split && !leftoverblock && !dict_training) {
    nstreams = (int32_t)typesize;
  }
  else {
    nstreams = 1;
  }
  neblock = bsize / nstreams;
  for (j = 0; j < nstreams; j++) {
    if (!dict_training) {
      dest += sizeof(int32_t);
      ntbytes += sizeof(int32_t);
      ctbytes += sizeof(int32_t);
    }
    const uint8_t* ip = (uint8_t*)_src + j * neblock;
    const uint8_t* ipbound = (uint8_t*)_src + (j + 1) * neblock;
    if (get_run(ip, ipbound)) {
      int32_t value = _src[j * neblock];
      _sw32(dest - 4, -value);
      continue;
    }
    maxout = neblock;
  #if defined(HAVE_SNAPPY)
    if (context->compcode == BLOSC_SNAPPY) {
      maxout = (int32_t)snappy_max_compressed_length((size_t)neblock);
    }
  #endif  
    if (ntbytes + maxout > maxbytes) {
      maxout = (int64_t)maxbytes - (int64_t)ntbytes;
      if (maxout <= 0) {
        return 0;                   
      }
    }
    if (dict_training) {
      memcpy(dest, _src + j * neblock, (unsigned int)neblock);
      cbytes = (int32_t)neblock;
    }
    else if (context->compcode == BLOSC_BLOSCLZ) {
      cbytes = blosclz_compress(context->clevel, _src + j * neblock,
                                (int)neblock, dest, (int)maxout);
    }
  #if defined(HAVE_LZ4)
    else if (context->compcode == BLOSC_LZ4) {
      void *hash_table = NULL;
    #ifdef HAVE_IPP
      hash_table = (void*)thread_context->lz4_hash_table;
    #endif
      cbytes = lz4_wrap_compress((char*)_src + j * neblock, (size_t)neblock,
                                 (char*)dest, (size_t)maxout, accel, hash_table);
    }
    else if (context->compcode == BLOSC_LZ4HC) {
      cbytes = lz4hc_wrap_compress((char*)_src + j * neblock, (size_t)neblock,
                                   (char*)dest, (size_t)maxout, context->clevel);
    }
  #endif  
  #if defined(HAVE_LIZARD)
    else if (context->compcode == BLOSC_LIZARD) {
      cbytes = lizard_wrap_compress((char*)_src + j * neblock, (size_t)neblock,
                                    (char*)dest, (size_t)maxout, accel);
    }
  #endif  
  #if defined(HAVE_SNAPPY)
    else if (context->compcode == BLOSC_SNAPPY) {
      cbytes = snappy_wrap_compress((char*)_src + j * neblock, (size_t)neblock,
                                    (char*)dest, (size_t)maxout);
    }
  #endif  
  #if defined(HAVE_ZLIB)
    else if (context->compcode == BLOSC_ZLIB) {
      cbytes = zlib_wrap_compress((char*)_src + j * neblock, (size_t)neblock,
                                  (char*)dest, (size_t)maxout, context->clevel);
    }
  #endif  
  #if defined(HAVE_ZSTD)
    else if (context->compcode == BLOSC_ZSTD) {
      cbytes = zstd_wrap_compress(thread_context,
                                  (char*)_src + j * neblock, (size_t)neblock,
                                  (char*)dest, (size_t)maxout, context->clevel);
    }
  #endif  
    else {
      blosc_compcode_to_compname(context->compcode, &compname);
      fprintf(stderr, "Blosc has not been compiled with '%s' ", compname);
      fprintf(stderr, "compression support.  Please use one having it.");
      return -5;     
    }
    if (cbytes > maxout) {
      return -1;
    }
    if (cbytes < 0) {
      return -2;
    }
    if (!dict_training) {
      if (cbytes == 0 || cbytes == neblock) {
        if ((ntbytes + neblock) > maxbytes) {
          return 0;     
        }
        memcpy(dest, _src + j * neblock, (unsigned int)neblock);
        cbytes = neblock;
      }
      _sw32(dest - 4, cbytes);
    }
    dest += cbytes;
    ntbytes += cbytes;
    ctbytes += cbytes;
  }   
  return ctbytes;
}