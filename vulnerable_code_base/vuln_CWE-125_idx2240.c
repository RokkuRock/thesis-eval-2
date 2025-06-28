static int cx24116_send_diseqc_msg(struct dvb_frontend *fe,
	struct dvb_diseqc_master_cmd *d)
{
	struct cx24116_state *state = fe->demodulator_priv;
	int i, ret;
	if (debug) {
		printk(KERN_INFO "cx24116: %s(", __func__);
		for (i = 0 ; i < d->msg_len ;) {
			printk(KERN_INFO "0x%02x", d->msg[i]);
			if (++i < d->msg_len)
				printk(KERN_INFO ", ");
		}
		printk(") toneburst=%d\n", toneburst);
	}
	if (d->msg_len > (CX24116_ARGLEN - CX24116_DISEQC_MSGOFS))
		return -EINVAL;
	for (i = 0; i < d->msg_len; i++)
		state->dsec_cmd.args[CX24116_DISEQC_MSGOFS + i] = d->msg[i];
	state->dsec_cmd.args[CX24116_DISEQC_MSGLEN] = d->msg_len;
	state->dsec_cmd.len = CX24116_DISEQC_MSGOFS +
		state->dsec_cmd.args[CX24116_DISEQC_MSGLEN];
	if (toneburst == CX24116_DISEQC_MESGCACHE)
		return 0;
	else if (toneburst == CX24116_DISEQC_TONEOFF)
		state->dsec_cmd.args[CX24116_DISEQC_BURST] = 0;
	else if (toneburst == CX24116_DISEQC_TONECACHE) {
		if (d->msg_len >= 4 && d->msg[2] == 0x38)
			state->dsec_cmd.args[CX24116_DISEQC_BURST] =
				((d->msg[3] & 4) >> 2);
		if (debug)
			dprintk("%s burst=%d\n", __func__,
				state->dsec_cmd.args[CX24116_DISEQC_BURST]);
	}
	ret = cx24116_wait_for_lnb(fe);
	if (ret != 0)
		return ret;
	msleep(100);
	ret = cx24116_cmd_execute(fe, &state->dsec_cmd);
	if (ret != 0)
		return ret;
	msleep((state->dsec_cmd.args[CX24116_DISEQC_MSGLEN] << 4) +
		((toneburst == CX24116_DISEQC_TONEOFF) ? 30 : 60));
	return 0;
}