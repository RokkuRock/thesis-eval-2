int dbFree(struct inode *ip, s64 blkno, s64 nblocks)
{
	struct metapage *mp;
	struct dmap *dp;
	int nb, rc;
	s64 lblkno, rem;
	struct inode *ipbmap = JFS_SBI(ip->i_sb)->ipbmap;
	struct bmap *bmp = JFS_SBI(ip->i_sb)->bmap;
	struct super_block *sb = ipbmap->i_sb;
	IREAD_LOCK(ipbmap, RDWRLOCK_DMAP);
	if (unlikely((blkno == 0) || (blkno + nblocks > bmp->db_mapsize))) {
		IREAD_UNLOCK(ipbmap);
		printk(KERN_ERR "blkno = %Lx, nblocks = %Lx\n",
		       (unsigned long long) blkno,
		       (unsigned long long) nblocks);
		jfs_error(ip->i_sb, "block to be freed is outside the map\n");
		return -EIO;
	}
	if (JFS_SBI(sb)->flag & JFS_DISCARD)
		if (JFS_SBI(sb)->minblks_trim <= nblocks)
			jfs_issue_discard(ipbmap, blkno, nblocks);
	mp = NULL;
	for (rem = nblocks; rem > 0; rem -= nb, blkno += nb) {
		if (mp) {
			write_metapage(mp);
		}
		lblkno = BLKTODMAP(blkno, bmp->db_l2nbperpage);
		mp = read_metapage(ipbmap, lblkno, PSIZE, 0);
		if (mp == NULL) {
			IREAD_UNLOCK(ipbmap);
			return -EIO;
		}
		dp = (struct dmap *) mp->data;
		nb = min(rem, BPERDMAP - (blkno & (BPERDMAP - 1)));
		if ((rc = dbFreeDmap(bmp, dp, blkno, nb))) {
			jfs_error(ip->i_sb, "error in block map\n");
			release_metapage(mp);
			IREAD_UNLOCK(ipbmap);
			return (rc);
		}
	}
	write_metapage(mp);
	IREAD_UNLOCK(ipbmap);
	return (0);
}