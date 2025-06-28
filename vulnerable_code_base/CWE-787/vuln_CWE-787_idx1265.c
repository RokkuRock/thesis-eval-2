static int directblockRead(struct READER *reader, struct DATAOBJECT *dataobject,
		struct FRACTALHEAP *fractalheap) {
	char buf[4], *name, *value;
	int size, offset_size, length_size, err, len;
	uint8_t typeandversion;
	uint64_t unknown, heap_header_address, block_offset, block_size, offset,
			length;
	long store;
	struct DIR *dir;
	struct MYSOFA_ATTRIBUTE *attr;
	UNUSED(offset);
	UNUSED(block_size);
	UNUSED(block_offset);
	if (fread(buf, 1, 4, reader->fhd) != 4 || strncmp(buf, "FHDB", 4)) {
		log("cannot read signature of fractal heap indirect block\n");
		return MYSOFA_INVALID_FORMAT;
	}
	log("%08" PRIX64 " %.4s\n", (uint64_t )ftell(reader->fhd) - 4, buf);
	if (fgetc(reader->fhd) != 0) {
		log("object FHDB must have version 0\n");
		return MYSOFA_UNSUPPORTED_FORMAT;
	}
	if (fseek(reader->fhd, reader->superblock.size_of_offsets, SEEK_CUR) < 0)
		return errno;
	size = (fractalheap->maximum_heap_size + 7) / 8;
	block_offset = readValue(reader, size);
	if (fractalheap->flags & 2)
		if (fseek(reader->fhd, 4, SEEK_CUR))
			return errno;
	offset_size = ceilf(log2f(fractalheap->maximum_heap_size) / 8);
	if (fractalheap->maximum_direct_block_size < fractalheap->maximum_size)
		length_size = ceilf(log2f(fractalheap->maximum_direct_block_size) / 8);
	else
		length_size = ceilf(log2f(fractalheap->maximum_size) / 8);
	log(" %d %" PRIu64 " %d\n",size,block_offset,offset_size);
	do {
		typeandversion = (uint8_t) fgetc(reader->fhd);
		offset = readValue(reader, offset_size);
		length = readValue(reader, length_size);
		if (offset > 0x10000000 || length > 0x10000000)
			return MYSOFA_UNSUPPORTED_FORMAT;
		log(" %d %4" PRIX64 " %" PRIX64 " %08lX\n",typeandversion,offset,length,ftell(reader->fhd));
		if (typeandversion == 3) {
			if (readValue(reader, 5) != 0x0000040008) {
				log("FHDB type 3 unsupported values");
				return MYSOFA_UNSUPPORTED_FORMAT;
			}
			if (!(name = malloc(length+1)))
				return MYSOFA_NO_MEMORY;
			if (fread(name, 1, length, reader->fhd) != length) {
				free(name);
				return MYSOFA_READ_ERROR;
			}
			name[length]=0;	
			if (readValue(reader, 4) != 0x00000013) {
				log("FHDB type 3 unsupported values");
				free(name);
				return MYSOFA_UNSUPPORTED_FORMAT;
			}
			len = (int) readValue(reader, 2);
			if (len > 0x1000 || len < 0) {
				free(name);
				return MYSOFA_UNSUPPORTED_FORMAT;
			}
			unknown = readValue(reader, 6);
			if (unknown == 0x000000020200)
				value = NULL;
			else if (unknown == 0x000000020000) {
				if (!(value = malloc(len + 1))) {
					free(name);
					return MYSOFA_NO_MEMORY;
				}
				if (fread(value, 1, len, reader->fhd) != len) {
					free(value);
					free(name);
					return MYSOFA_READ_ERROR;
				}
				value[len] = 0;
			} else if (unknown == 0x20000020000) {
				if (!(value = malloc(5))) {
					free(name);
					return MYSOFA_NO_MEMORY;
				}
				strcpy(value, "");
			} else {
				log("FHDB type 3 unsupported values: %12" PRIX64 "\n",unknown);
				free(name);
				return MYSOFA_OK;
			} 
			log(" %s = %s\n", name, value);
			attr = malloc(sizeof(struct MYSOFA_ATTRIBUTE));
			attr->name = name;
			attr->value = value;
			attr->next = dataobject->attributes;
			dataobject->attributes = attr;
		} else if (typeandversion == 1) {
			unknown = readValue(reader, 6);
			if (unknown) {
				log("FHDB type 1 unsupported values\n");
				return MYSOFA_UNSUPPORTED_FORMAT;
			}
			len = fgetc(reader->fhd);
			if (len < 0)
				return MYSOFA_READ_ERROR;
			assert(len < 0x100);
			if (!(name = malloc(len + 1)))
				return MYSOFA_NO_MEMORY;
			if (fread(name, 1, len, reader->fhd) != len) {
				free(name);
				return MYSOFA_READ_ERROR;
			}
			name[len] = 0;
			heap_header_address = readValue(reader,
					reader->superblock.size_of_offsets);
			log("\nfractal head type 1 length %4" PRIX64 " name %s address %" PRIX64 "\n", length, name, heap_header_address);
			dir = malloc(sizeof(struct DIR));
			if (!dir) {
				free(name);
				return MYSOFA_NO_MEMORY;
			}
			memset(dir, 0, sizeof(*dir));
			dir->next = dataobject->directory;
			dataobject->directory = dir;
			store = ftell(reader->fhd);
			if (fseek(reader->fhd, heap_header_address, SEEK_SET)) {
				free(name);
				return errno;
			}
			err = dataobjectRead(reader, &dir->dataobject, name);
			if (err) {
				return err;
			}
			if (store < 0) {
				return errno;
			}
			if (fseek(reader->fhd, store, SEEK_SET) < 0)
				return errno;
		} else if (typeandversion != 0) {
			log("fractal head unknown type %d\n", typeandversion);
			return MYSOFA_OK;
		}
	} while (typeandversion != 0);
	return MYSOFA_OK;
}