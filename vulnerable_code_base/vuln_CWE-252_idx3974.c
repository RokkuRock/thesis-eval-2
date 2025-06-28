pci_vtrnd_notify(void *vsc, struct vqueue_info *vq)
{
	struct iovec iov;
	struct pci_vtrnd_softc *sc;
	int len;
	uint16_t idx;
	sc = vsc;
	if (sc->vrsc_fd < 0) {
		vq_endchains(vq, 0);
		return;
	}
	while (vq_has_descs(vq)) {
		vq_getchain(vq, &idx, &iov, 1, NULL);
		len = (int) read(sc->vrsc_fd, iov.iov_base, iov.iov_len);
		DPRINTF(("vtrnd: vtrnd_notify(): %d\r\n", len));
		assert(len > 0);
		vq_relchain(vq, idx, (uint32_t)len);
	}
	vq_endchains(vq, 1);	 
}