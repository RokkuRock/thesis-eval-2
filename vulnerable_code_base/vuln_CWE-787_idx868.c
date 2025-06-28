static int do_i2c_write(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	uint	chip;
	uint	devaddr, length;
	int alen;
	u_char  *memaddr;
	int ret;
#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;
	struct dm_i2c_chip *i2c_chip;
#endif
	if ((argc < 5) || (argc > 6))
		return cmd_usage(cmdtp);
	memaddr = (u_char *)hextoul(argv[1], NULL);
	chip = hextoul(argv[2], NULL);
	devaddr = hextoul(argv[3], NULL);
	alen = get_alen(argv[3], DEFAULT_ADDR_LEN);
	if (alen > 3)
		return cmd_usage(cmdtp);
	length = hextoul(argv[4], NULL);
#if CONFIG_IS_ENABLED(DM_I2C)
	ret = i2c_get_cur_bus_chip(chip, &dev);
	if (!ret && alen != -1)
		ret = i2c_set_chip_offset_len(dev, alen);
	if (ret)
		return i2c_report_err(ret, I2C_ERR_WRITE);
	i2c_chip = dev_get_parent_plat(dev);
	if (!i2c_chip)
		return i2c_report_err(ret, I2C_ERR_WRITE);
#endif
	if (argc == 6 && !strcmp(argv[5], "-s")) {
#if CONFIG_IS_ENABLED(DM_I2C)
		i2c_chip->flags &= ~DM_I2C_CHIP_WR_ADDRESS;
		ret = dm_i2c_write(dev, devaddr, memaddr, length);
#else
		ret = i2c_write(chip, devaddr, alen, memaddr, length);
#endif
		if (ret)
			return i2c_report_err(ret, I2C_ERR_WRITE);
	} else {
		while (length-- > 0) {
#if CONFIG_IS_ENABLED(DM_I2C)
			i2c_chip->flags |= DM_I2C_CHIP_WR_ADDRESS;
			ret = dm_i2c_write(dev, devaddr++, memaddr++, 1);
#else
			ret = i2c_write(chip, devaddr++, alen, memaddr++, 1);
#endif
			if (ret)
				return i2c_report_err(ret, I2C_ERR_WRITE);
#if !defined(CONFIG_SYS_I2C_FRAM)
			udelay(11000);
#endif
		}
	}
	return 0;
}