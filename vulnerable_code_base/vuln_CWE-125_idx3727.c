INST_HANDLER (sbrx) {	 
	int b = buf[0] & 0x7;
	int r = ((buf[0] >> 4) & 0xf) | ((buf[1] & 0x01) << 4);
	RAnalOp next_op;
	avr_op_analyze (anal,
			&next_op,
			op->addr + op->size, buf + op->size, len - op->size,
			cpu);
	r_strbuf_fini (&next_op.esil);
	op->jump = op->addr + next_op.size + 2;
	op->cycles = 1;	 
	ESIL_A ("%d,1,<<,r%d,&,", b, r);			 
	ESIL_A ((buf[1] & 0xe) == 0xc
			? "!,"		 
			: "!,!,");	 
	ESIL_A ("?{,%"PFMT64d",pc,=,},", op->jump);	 
}