pci_vtblk_proc(struct pci_vtblk_softc *sc, struct vqueue_info *vq)
{
	struct virtio_blk_hdr *vbh;
	struct virtio_blk_discard_write_zeroes *vbdiscard;
	struct pci_vtblk_ioreq *io;
	int i, n;
	int err;
	ssize_t iolen;
	int expectro, type;
	struct iovec iov[BLOCKIF_IOV_MAX + 2];
	uint16_t idx, flags[BLOCKIF_IOV_MAX + 2];
	n = vq_getchain(vq, &idx, iov, BLOCKIF_IOV_MAX + 2, flags);
	assert(n >= 2 && n <= BLOCKIF_IOV_MAX + 2);
	io = &sc->vbsc_ios[idx];
	assert((flags[0] & VRING_DESC_F_WRITE) == 0);
	assert(iov[0].iov_len == sizeof(struct virtio_blk_hdr));
	vbh = iov[0].iov_base;
	memcpy(&io->io_req.br_iov, &iov[1],
	       sizeof(struct iovec) * ((size_t)n - 2));
	io->io_req.br_iovcnt = n - 2;
	io->io_req.br_offset = (off_t)(vbh->vbh_sector * DEV_BSIZE);
	io->io_status = iov[--n].iov_base;
	assert(iov[n].iov_len == 1);
	assert(flags[n] & VRING_DESC_F_WRITE);
	type = vbh->vbh_type & ~VBH_FLAG_BARRIER;
	expectro = (type == VBH_OP_WRITE) || (type == VBH_OP_DISCARD);
	iolen = 0;
	for (i = 1; i < n; i++) {
		assert(((flags[i] & VRING_DESC_F_WRITE) == 0) == expectro);
		iolen += iov[i].iov_len;
	}
	io->io_req.br_resid = iolen;
	DPRINTF(("virtio-block: %s op, %zd bytes, %d segs\n\r",
		 print_vbh_op(type), iolen, i - 1));
	switch (type) {
	case VBH_OP_READ:
		err = blockif_read(sc->bc, &io->io_req);
		break;
	case VBH_OP_WRITE:
		err = blockif_write(sc->bc, &io->io_req);
		break;
	case VBH_OP_DISCARD:
		assert(iov[1].iov_len = sizeof(struct virtio_blk_discard_write_zeroes));
		assert(n == 2);
		vbdiscard = iov[1].iov_base;
		io->io_req.br_offset = (off_t) vbdiscard->sector * DEV_BSIZE;
		io->io_req.br_resid = vbdiscard->num_sectors * DEV_BSIZE;
		err = blockif_delete(sc->bc, &io->io_req);
		break;
	case VBH_OP_FLUSH:
	case VBH_OP_FLUSH_OUT:
		err = blockif_flush(sc->bc, &io->io_req);
		break;
	case VBH_OP_IDENT:
		memset(iov[1].iov_base, 0, iov[1].iov_len);
		strncpy(iov[1].iov_base, sc->vbsc_ident,
		    MIN(iov[1].iov_len, sizeof(sc->vbsc_ident)));
		pci_vtblk_done_locked(&io->io_req, 0);
		return;
	default:
		pci_vtblk_done_locked(&io->io_req, EOPNOTSUPP);
		return;
	}
	assert(err == 0);
}