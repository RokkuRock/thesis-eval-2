int parse_rock_ridge_inode(struct iso_directory_record *de, struct inode *inode)
{
	int result = parse_rock_ridge_inode_internal(de, inode, 0);
	if ((ISOFS_SB(inode->i_sb)->s_rock_offset == -1)
	    && (ISOFS_SB(inode->i_sb)->s_rock == 2)) {
		result = parse_rock_ridge_inode_internal(de, inode, 14);
	}
	return result;
}