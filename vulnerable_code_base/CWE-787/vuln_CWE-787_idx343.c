static int do_i2c_read(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	uint	chip;
	uint	devaddr, length;
	int alen;
	u_char  *memaddr;
	int ret;
#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;
#endif
	if (argc != 5)
		return CMD_RET_USAGE;
	chip = hextoul(argv[1], NULL);
	devaddr = hextoul(argv[2], NULL);
	alen = get_alen(argv[2], DEFAULT_ADDR_LEN);
	if (alen > 3)
		return CMD_RET_USAGE;
	length = hextoul(argv[3], NULL);
	memaddr = (u_char *)hextoul(argv[4], NULL);
#if CONFIG_IS_ENABLED(DM_I2C)
	ret = i2c_get_cur_bus_chip(chip, &dev);
	if (!ret && alen != -1)
		ret = i2c_set_chip_offset_len(dev, alen);
	if (!ret)
		ret = dm_i2c_read(dev, devaddr, memaddr, length);
#else
	ret = i2c_read(chip, devaddr, alen, memaddr, length);
#endif
	if (ret)
		return i2c_report_err(ret, I2C_ERR_READ);
	return 0;
}