int saa7164_bus_get(struct saa7164_dev *dev, struct tmComResInfo* msg,
	void *buf, int peekonly)
{
	struct tmComResBusInfo *bus = &dev->bus;
	u32 bytes_to_read, write_distance, curr_grp, curr_gwp,
		new_grp, buf_size, space_rem;
	struct tmComResInfo msg_tmp;
	int ret = SAA_ERR_BAD_PARAMETER;
	saa7164_bus_verify(dev);
	if (msg == NULL)
		return ret;
	if (msg->size > dev->bus.m_wMaxReqSize) {
		printk(KERN_ERR "%s() Exceeded dev->bus.m_wMaxReqSize\n",
			__func__);
		return ret;
	}
	if ((peekonly == 0) && (msg->size > 0) && (buf == NULL)) {
		printk(KERN_ERR
			"%s() Missing msg buf, size should be %d bytes\n",
			__func__, msg->size);
		return ret;
	}
	mutex_lock(&bus->lock);
	curr_gwp = saa7164_readl(bus->m_dwGetWritePos);
	curr_grp = saa7164_readl(bus->m_dwGetReadPos);
	if (curr_gwp == curr_grp) {
		ret = SAA_ERR_EMPTY;
		goto out;
	}
	bytes_to_read = sizeof(*msg);
	write_distance = 0;
	if (curr_gwp >= curr_grp)
		write_distance = curr_gwp - curr_grp;
	else
		write_distance = curr_gwp + bus->m_dwSizeGetRing - curr_grp;
	if (bytes_to_read > write_distance) {
		printk(KERN_ERR "%s() No message/response found\n", __func__);
		ret = SAA_ERR_INVALID_COMMAND;
		goto out;
	}
	new_grp = curr_grp + bytes_to_read;
	if (new_grp > bus->m_dwSizeGetRing) {
		new_grp -= bus->m_dwSizeGetRing;
		space_rem = bus->m_dwSizeGetRing - curr_grp;
		memcpy_fromio(&msg_tmp, bus->m_pdwGetRing + curr_grp, space_rem);
		memcpy_fromio((u8 *)&msg_tmp + space_rem, bus->m_pdwGetRing,
			bytes_to_read - space_rem);
	} else {
		memcpy_fromio(&msg_tmp, bus->m_pdwGetRing + curr_grp, bytes_to_read);
	}
	msg_tmp.size = le16_to_cpu((__force __le16)msg_tmp.size);
	msg_tmp.command = le32_to_cpu((__force __le32)msg_tmp.command);
	msg_tmp.controlselector = le16_to_cpu((__force __le16)msg_tmp.controlselector);
	if (peekonly) {
		memcpy(msg, &msg_tmp, sizeof(*msg));
		goto peekout;
	}
	if ((msg_tmp.id != msg->id) || (msg_tmp.command != msg->command) ||
		(msg_tmp.controlselector != msg->controlselector) ||
		(msg_tmp.seqno != msg->seqno) || (msg_tmp.size != msg->size)) {
		printk(KERN_ERR "%s() Unexpected msg miss-match\n", __func__);
		saa7164_bus_dumpmsg(dev, msg, buf);
		saa7164_bus_dumpmsg(dev, &msg_tmp, NULL);
		ret = SAA_ERR_INVALID_COMMAND;
		goto out;
	}
	buf_size = msg->size;
	bytes_to_read = sizeof(*msg) + msg->size;
	write_distance = 0;
	if (curr_gwp >= curr_grp)
		write_distance = curr_gwp - curr_grp;
	else
		write_distance = curr_gwp + bus->m_dwSizeGetRing - curr_grp;
	if (bytes_to_read > write_distance) {
		printk(KERN_ERR "%s() Invalid bus state, missing msg or mangled ring, faulty H/W / bad code?\n",
		       __func__);
		ret = SAA_ERR_INVALID_COMMAND;
		goto out;
	}
	new_grp = curr_grp + bytes_to_read;
	if (new_grp > bus->m_dwSizeGetRing) {
		new_grp -= bus->m_dwSizeGetRing;
		space_rem = bus->m_dwSizeGetRing - curr_grp;
		if (space_rem < sizeof(*msg)) {
			memcpy_fromio(msg, bus->m_pdwGetRing + curr_grp, space_rem);
			memcpy_fromio((u8 *)msg + space_rem, bus->m_pdwGetRing,
				sizeof(*msg) - space_rem);
			if (buf)
				memcpy_fromio(buf, bus->m_pdwGetRing + sizeof(*msg) -
					space_rem, buf_size);
		} else if (space_rem == sizeof(*msg)) {
			memcpy_fromio(msg, bus->m_pdwGetRing + curr_grp, sizeof(*msg));
			if (buf)
				memcpy_fromio(buf, bus->m_pdwGetRing, buf_size);
		} else {
			memcpy_fromio(msg, bus->m_pdwGetRing + curr_grp, sizeof(*msg));
			if (buf) {
				memcpy_fromio(buf, bus->m_pdwGetRing + curr_grp +
					sizeof(*msg), space_rem - sizeof(*msg));
				memcpy_fromio(buf + space_rem - sizeof(*msg),
					bus->m_pdwGetRing, bytes_to_read -
					space_rem);
			}
		}
	} else {
		memcpy_fromio(msg, bus->m_pdwGetRing + curr_grp, sizeof(*msg));
		if (buf)
			memcpy_fromio(buf, bus->m_pdwGetRing + curr_grp + sizeof(*msg),
				buf_size);
	}
	msg->size = le16_to_cpu((__force __le16)msg->size);
	msg->command = le32_to_cpu((__force __le32)msg->command);
	msg->controlselector = le16_to_cpu((__force __le16)msg->controlselector);
	saa7164_writel(bus->m_dwGetReadPos, new_grp);
peekout:
	ret = SAA_OK;
out:
	mutex_unlock(&bus->lock);
	saa7164_bus_verify(dev);
	return ret;
}