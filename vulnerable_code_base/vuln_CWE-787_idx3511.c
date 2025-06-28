static void set_ntacl_dacl(struct user_namespace *user_ns,
			   struct smb_acl *pndacl,
			   struct smb_acl *nt_dacl,
			   const struct smb_sid *pownersid,
			   const struct smb_sid *pgrpsid,
			   struct smb_fattr *fattr)
{
	struct smb_ace *ntace, *pndace;
	int nt_num_aces = le32_to_cpu(nt_dacl->num_aces), num_aces = 0;
	unsigned short size = 0;
	int i;
	pndace = (struct smb_ace *)((char *)pndacl + sizeof(struct smb_acl));
	if (nt_num_aces) {
		ntace = (struct smb_ace *)((char *)nt_dacl + sizeof(struct smb_acl));
		for (i = 0; i < nt_num_aces; i++) {
			memcpy((char *)pndace + size, ntace, le16_to_cpu(ntace->size));
			size += le16_to_cpu(ntace->size);
			ntace = (struct smb_ace *)((char *)ntace + le16_to_cpu(ntace->size));
			num_aces++;
		}
	}
	set_posix_acl_entries_dacl(user_ns, pndace, fattr,
				   &num_aces, &size, nt_num_aces);
	pndacl->num_aces = cpu_to_le32(num_aces);
	pndacl->size = cpu_to_le16(le16_to_cpu(pndacl->size) + size);
}