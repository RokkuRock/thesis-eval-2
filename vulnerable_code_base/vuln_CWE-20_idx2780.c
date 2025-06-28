static int parse_exports_table(long long *table_start)
{
	int res;
	int indexes = SQUASHFS_LOOKUP_BLOCKS(sBlk.s.inodes);
	long long export_index_table[indexes];
	res = read_fs_bytes(fd, sBlk.s.lookup_table_start,
		SQUASHFS_LOOKUP_BLOCK_BYTES(sBlk.s.inodes), export_index_table);
	if(res == FALSE) {
		ERROR("parse_exports_table: failed to read export index table\n");
		return FALSE;
	}
	SQUASHFS_INSWAP_LOOKUP_BLOCKS(export_index_table, indexes);
	*table_start = export_index_table[0];
	return TRUE;
}