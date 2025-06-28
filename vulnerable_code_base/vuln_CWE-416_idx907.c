int ipmi_si_mem_setup(struct si_sm_io *io)
{
	unsigned long addr = io->addr_data;
	int           mapsize, idx;
	if (!addr)
		return -ENODEV;
	io->io_cleanup = mem_cleanup;
	switch (io->regsize) {
	case 1:
		io->inputb = intf_mem_inb;
		io->outputb = intf_mem_outb;
		break;
	case 2:
		io->inputb = intf_mem_inw;
		io->outputb = intf_mem_outw;
		break;
	case 4:
		io->inputb = intf_mem_inl;
		io->outputb = intf_mem_outl;
		break;
#ifdef readq
	case 8:
		io->inputb = mem_inq;
		io->outputb = mem_outq;
		break;
#endif
	default:
		dev_warn(io->dev, "Invalid register size: %d\n",
			 io->regsize);
		return -EINVAL;
	}
	for (idx = 0; idx < io->io_size; idx++) {
		if (request_mem_region(addr + idx * io->regspacing,
				       io->regsize, DEVICE_NAME) == NULL) {
			mem_region_cleanup(io, idx);
			return -EIO;
		}
	}
	mapsize = ((io->io_size * io->regspacing)
		   - (io->regspacing - io->regsize));
	io->addr = ioremap(addr, mapsize);
	if (io->addr == NULL) {
		mem_region_cleanup(io, io->io_size);
		return -EIO;
	}
	return 0;
}