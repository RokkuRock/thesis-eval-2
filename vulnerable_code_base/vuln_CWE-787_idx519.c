int build_sec_desc(struct user_namespace *user_ns,
		   struct smb_ntsd *pntsd, struct smb_ntsd *ppntsd,
		   int addition_info, __u32 *secdesclen,
		   struct smb_fattr *fattr)
{
	int rc = 0;
	__u32 offset;
	struct smb_sid *owner_sid_ptr, *group_sid_ptr;
	struct smb_sid *nowner_sid_ptr, *ngroup_sid_ptr;
	struct smb_acl *dacl_ptr = NULL;  
	uid_t uid;
	gid_t gid;
	unsigned int sid_type = SIDOWNER;
	nowner_sid_ptr = kmalloc(sizeof(struct smb_sid), GFP_KERNEL);
	if (!nowner_sid_ptr)
		return -ENOMEM;
	uid = from_kuid(&init_user_ns, fattr->cf_uid);
	if (!uid)
		sid_type = SIDUNIX_USER;
	id_to_sid(uid, sid_type, nowner_sid_ptr);
	ngroup_sid_ptr = kmalloc(sizeof(struct smb_sid), GFP_KERNEL);
	if (!ngroup_sid_ptr) {
		kfree(nowner_sid_ptr);
		return -ENOMEM;
	}
	gid = from_kgid(&init_user_ns, fattr->cf_gid);
	id_to_sid(gid, SIDUNIX_GROUP, ngroup_sid_ptr);
	offset = sizeof(struct smb_ntsd);
	pntsd->sacloffset = 0;
	pntsd->revision = cpu_to_le16(1);
	pntsd->type = cpu_to_le16(SELF_RELATIVE);
	if (ppntsd)
		pntsd->type |= ppntsd->type;
	if (addition_info & OWNER_SECINFO) {
		pntsd->osidoffset = cpu_to_le32(offset);
		owner_sid_ptr = (struct smb_sid *)((char *)pntsd + offset);
		smb_copy_sid(owner_sid_ptr, nowner_sid_ptr);
		offset += 1 + 1 + 6 + (nowner_sid_ptr->num_subauth * 4);
	}
	if (addition_info & GROUP_SECINFO) {
		pntsd->gsidoffset = cpu_to_le32(offset);
		group_sid_ptr = (struct smb_sid *)((char *)pntsd + offset);
		smb_copy_sid(group_sid_ptr, ngroup_sid_ptr);
		offset += 1 + 1 + 6 + (ngroup_sid_ptr->num_subauth * 4);
	}
	if (addition_info & DACL_SECINFO) {
		pntsd->type |= cpu_to_le16(DACL_PRESENT);
		dacl_ptr = (struct smb_acl *)((char *)pntsd + offset);
		dacl_ptr->revision = cpu_to_le16(2);
		dacl_ptr->size = cpu_to_le16(sizeof(struct smb_acl));
		dacl_ptr->num_aces = 0;
		if (!ppntsd) {
			set_mode_dacl(user_ns, dacl_ptr, fattr);
		} else if (!ppntsd->dacloffset) {
			goto out;
		} else {
			struct smb_acl *ppdacl_ptr;
			ppdacl_ptr = (struct smb_acl *)((char *)ppntsd +
						le32_to_cpu(ppntsd->dacloffset));
			set_ntacl_dacl(user_ns, dacl_ptr, ppdacl_ptr,
				       nowner_sid_ptr, ngroup_sid_ptr, fattr);
		}
		pntsd->dacloffset = cpu_to_le32(offset);
		offset += le16_to_cpu(dacl_ptr->size);
	}
out:
	kfree(nowner_sid_ptr);
	kfree(ngroup_sid_ptr);
	*secdesclen = offset;
	return rc;
}