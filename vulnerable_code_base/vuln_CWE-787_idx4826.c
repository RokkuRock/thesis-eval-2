static int do_i2c_mw(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	uint	chip;
	ulong	addr;
	int	alen;
	uchar	byte;
	int	count;
	int ret;
#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;
#endif
	if ((argc < 4) || (argc > 5))
		return CMD_RET_USAGE;
	chip = hextoul(argv[1], NULL);
	addr = hextoul(argv[2], NULL);
	alen = get_alen(argv[2], DEFAULT_ADDR_LEN);
	if (alen > 3)
		return CMD_RET_USAGE;
#if CONFIG_IS_ENABLED(DM_I2C)
	ret = i2c_get_cur_bus_chip(chip, &dev);
	if (!ret && alen != -1)
		ret = i2c_set_chip_offset_len(dev, alen);
	if (ret)
		return i2c_report_err(ret, I2C_ERR_WRITE);
#endif
	byte = hextoul(argv[3], NULL);
	if (argc == 5)
		count = hextoul(argv[4], NULL);
	else
		count = 1;
	while (count-- > 0) {
#if CONFIG_IS_ENABLED(DM_I2C)
		ret = dm_i2c_write(dev, addr++, &byte, 1);
#else
		ret = i2c_write(chip, addr++, alen, &byte, 1);
#endif
		if (ret)
			return i2c_report_err(ret, I2C_ERR_WRITE);
#if !defined(CONFIG_SYS_I2C_FRAM)
		udelay(11000);
#endif
	}
	return 0;
}