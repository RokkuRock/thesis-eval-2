int read_xattrs_from_disk(int fd, struct squashfs_super_block *sBlk, int flag, long long *table_start)
{
	int res, bytes, i, indexes, index_bytes, ids;
	long long *index, start, end;
	struct squashfs_xattr_table id_table;
	TRACE("read_xattrs_from_disk\n");
	if(sBlk->xattr_id_table_start == SQUASHFS_INVALID_BLK)
		return SQUASHFS_INVALID_BLK;
	res = read_fs_bytes(fd, sBlk->xattr_id_table_start, sizeof(id_table),
		&id_table);
	if(res == 0)
		return 0;
	SQUASHFS_INSWAP_XATTR_TABLE(&id_table);
	if(flag) {
		*table_start = id_table.xattr_table_start;
		return id_table.xattr_ids;
	}
	ids = id_table.xattr_ids;
	xattr_table_start = id_table.xattr_table_start;
	index_bytes = SQUASHFS_XATTR_BLOCK_BYTES(ids);
	indexes = SQUASHFS_XATTR_BLOCKS(ids);
	index = malloc(index_bytes);
	if(index == NULL)
		MEM_ERROR();
	res = read_fs_bytes(fd, sBlk->xattr_id_table_start + sizeof(id_table),
		index_bytes, index);
	if(res ==0)
		goto failed1;
	SQUASHFS_INSWAP_LONG_LONGS(index, indexes);
	bytes = SQUASHFS_XATTR_BYTES(ids);
	xattr_ids = malloc(bytes);
	if(xattr_ids == NULL)
		MEM_ERROR();
	for(i = 0; i < indexes; i++) {
		int expected = (i + 1) != indexes ? SQUASHFS_METADATA_SIZE :
					bytes & (SQUASHFS_METADATA_SIZE - 1);
		int length = read_block(fd, index[i], NULL, expected,
			((unsigned char *) xattr_ids) +
			(i * SQUASHFS_METADATA_SIZE));
		TRACE("Read xattr id table block %d, from 0x%llx, length "
			"%d\n", i, index[i], length);
		if(length == 0) {
			ERROR("Failed to read xattr id table block %d, "
				"from 0x%llx, length %d\n", i, index[i],
				length);
			goto failed2;
		}
	}
	start = xattr_table_start;
	end = index[0];
	for(i = 0; start < end; i++) {
		int length;
		xattrs = realloc(xattrs, (i + 1) * SQUASHFS_METADATA_SIZE);
		if(xattrs == NULL)
			MEM_ERROR();
		save_xattr_block(start, i * SQUASHFS_METADATA_SIZE);
		length = read_block(fd, start, &start, 0,
			((unsigned char *) xattrs) +
			(i * SQUASHFS_METADATA_SIZE));
		TRACE("Read xattr block %d, length %d\n", i, length);
		if(length == 0) {
			ERROR("Failed to read xattr block %d\n", i);
			goto failed3;
		}
		if(start != end && length != SQUASHFS_METADATA_SIZE) {
			ERROR("Xattr block %d should be %d bytes in length, "
				"it is %d bytes\n", i, SQUASHFS_METADATA_SIZE,
				length);
			goto failed3;
		}
	}
	for(i = 0; i < ids; i++)
		SQUASHFS_INSWAP_XATTR_ID(&xattr_ids[i]);
	free(index);
	return ids;
failed3:
	free(xattrs);
failed2:
	free(xattr_ids);
failed1:
	free(index);
	return 0;
}