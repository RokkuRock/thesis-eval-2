read_fru_area(struct ipmi_intf * intf, struct fru_info *fru, uint8_t id,
			uint32_t offset, uint32_t length, uint8_t *frubuf)
{
	uint32_t off = offset, tmp, finish;
	struct ipmi_rs * rsp;
	struct ipmi_rq req;
	uint8_t msg_data[4];
	if (offset > fru->size) {
		lprintf(LOG_ERR, "Read FRU Area offset incorrect: %d > %d",
			offset, fru->size);
		return -1;
	}
	finish = offset + length;
	if (finish > fru->size) {
		finish = fru->size;
		lprintf(LOG_NOTICE, "Read FRU Area length %d too large, "
			"Adjusting to %d",
			offset + length, finish - offset);
	}
	memset(&req, 0, sizeof(req));
	req.msg.netfn = IPMI_NETFN_STORAGE;
	req.msg.cmd = GET_FRU_DATA;
	req.msg.data = msg_data;
	req.msg.data_len = 4;
	if (fru->max_read_size == 0) {
		uint16_t max_rs_size = ipmi_intf_get_max_response_data_size(intf) - 1;
		if (max_rs_size <= 1) {
			lprintf(LOG_ERROR, "Maximum response size is too small to send "
					"a read request");
			return -1;
		}
		if (max_rs_size - 1 > 255) {
			fru->max_read_size = 255;
		} else {
			fru->max_read_size = max_rs_size - 1;
		}
		if (fru->access) {
			fru->max_read_size &= ~1;
		}
	}
	do {
		tmp = fru->access ? off >> 1 : off;
		msg_data[0] = id;
		msg_data[1] = (uint8_t)(tmp & 0xff);
		msg_data[2] = (uint8_t)(tmp >> 8);
		tmp = finish - off;
		if (tmp > fru->max_read_size)
			msg_data[3] = (uint8_t)fru->max_read_size;
		else
			msg_data[3] = (uint8_t)tmp;
		rsp = intf->sendrecv(intf, &req);
		if (!rsp) {
			lprintf(LOG_NOTICE, "FRU Read failed");
			break;
		}
		if (rsp->ccode) {
			if (fru_cc_rq2big(rsp->ccode)
			    && fru->max_read_size > FRU_BLOCK_SZ)
			{
				if (fru->max_read_size > FRU_AREA_MAXIMUM_BLOCK_SZ) {
					fru->max_read_size -= FRU_BLOCK_SZ;
				} else {
					fru->max_read_size--;
				}
				lprintf(LOG_INFO, "Retrying FRU read with request size %d",
						fru->max_read_size);
				continue;
			}
			lprintf(LOG_NOTICE, "FRU Read failed: %s",
				val2str(rsp->ccode, completion_code_vals));
			break;
		}
		tmp = fru->access ? rsp->data[0] << 1 : rsp->data[0];
		memcpy(frubuf, rsp->data + 1, tmp);
		off += tmp;
		frubuf += tmp;
		if (tmp == 0 && off < finish) {
			return 0;
		}
	} while (off < finish);
	if (off < finish) {
		return -1;
	}
	return 0;
}