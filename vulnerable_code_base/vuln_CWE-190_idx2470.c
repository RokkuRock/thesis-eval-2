void bn_rec_tnaf_get(uint8_t *t, int8_t *beta, int8_t *gama, int8_t u, int w) {
	if (u == -1) {
		switch (w) {
			case 2:
			case 3:
				*t = 2;
				break;
			case 4:
				*t = 10;
				break;
			case 5:
			case 6:
				*t = 26;
				break;
			case 7:
			case 8:
				*t = 90;
				break;
		}
	} else {
		switch (w) {
			case 2:
				*t = 2;
				break;
			case 3:
			case 4:
			case 5:
				*t = 6;
				break;
			case 6:
			case 7:
				*t = 38;
				break;
			case 8:
				*t = 166;
				break;
		}
	}
	beta[0] = 1;
	gama[0] = 0;
	if (w >= 3) {
		beta[1] = 1;
		gama[1] = (int8_t)-u;
	}
	if (w >= 4) {
		beta[1] = -3;
		beta[2] = -1;
		beta[3] = 1;
		gama[1] = gama[2] = gama[3] = (int8_t)u;
	}
	if (w >= 5) {
		beta[4] = -3;
		beta[5] = -1;
		beta[6] = beta[7] = 1;
		gama[4] = gama[5] = gama[6] = (int8_t)(2 * u);
		gama[7] = (int8_t)(-3 * u);
	}
	if (w >= 6) {
		beta[1] = beta[8] = beta[14] = 3;
		beta[2] = beta[9] = beta[15] = 5;
		beta[3] = -5;
		beta[4] = beta[10] = beta[11] = -3;
		beta[5] = beta[12] = -1;
		beta[6] = beta[7] = beta[13] = 1;
		gama[1] = gama[2] = 0;
		gama[3] = gama[4] = gama[5] = gama[6] = (int8_t)(2 * u);
		gama[7] = gama[8] = gama[9] = (int8_t)(-3 * u);
		gama[10] = (int8_t)(4 * u);
		gama[11] = gama[12] = gama[13] = (int8_t)(-u);
		gama[14] = gama[15] = (int8_t)(-u);
	}
	if (w >= 7) {
		beta[3] = beta[22] = beta[29] = 7;
		beta[4] = beta[16] = beta[23] = -5;
		beta[5] = beta[10] = beta[17] = beta[24] = -3;
		beta[6] = beta[11] = beta[18] = beta[25] = beta[30] = -1;
		beta[7] = beta[12] = beta[14] = beta[19] = beta[26] = beta[31] = 1;
		beta[8] = beta[13] = beta[20] = beta[27] = 3;
		beta[9] = beta[21] = beta[28] = 5;
		beta[15] = -7;
		gama[3] = 0;
		gama[4] = gama[5] = gama[6] = (int8_t)(-3 * u);
		gama[11] = gama[12] = gama[13] = (int8_t)(4 * u);
		gama[14] = (int8_t)(-6 * u);
		gama[15] = gama[16] = gama[17] = gama[18] = (int8_t)u;
		gama[19] = gama[20] = gama[21] = gama[22] = (int8_t)u;
		gama[23] = gama[24] = gama[25] = gama[26] = (int8_t)(-2 * u);
		gama[27] = gama[28] = gama[29] = (int8_t)(-2 * u);
		gama[30] = gama[31] = (int8_t)(5 * u);
	}
	if (w == 8) {
		beta[10] = beta[17] = beta[48] = beta[55] = beta[62] = 7;
		beta[11] = beta[18] = beta[49] = beta[56] = beta[63] = 9;
		beta[12] = beta[22] = beta[29] = -3;
		beta[36] = beta[43] = beta[50] = -3;
		beta[13] = beta[23] = beta[30] = beta[37] = -1;
		beta[44] = beta[51] = beta[58] = -1;
		beta[14] = beta[24] = beta[31] = beta[38] = 1;
		beta[45] = beta[52] = beta[59] = 1;
		beta[15] = beta[32] = beta[39] = beta[46] = beta[53] = beta[60] = 3;
		beta[16] = beta[40] = beta[47] = beta[54] = beta[61] = 5;
		beta[19] = beta[57] = 11;
		beta[20] = beta[27] = beta[34] = beta[41] = -7;
		beta[21] = beta[28] = beta[35] = beta[42] = -5;
		beta[25] = -11;
		beta[26] = beta[33] = -9;
		gama[10] = gama[11] = (int8_t)(-3 * u);
		gama[12] = gama[13] = gama[14] = gama[15] = (int8_t)(-6 * u);
		gama[16] = gama[17] = gama[18] = gama[19] = (int8_t)(-6 * u);
		gama[20] = gama[21] = gama[22] = (int8_t)(8 * u);
		gama[23] = gama[24] = (int8_t)(8 * u);
		gama[25] = gama[26] = gama[27] = gama[28] = (int8_t)(5 * u);
		gama[29] = gama[30] = gama[31] = gama[32] = (int8_t)(5 * u);
		gama[33] = gama[34] = gama[35] = gama[36] = (int8_t)(2 * u);
		gama[37] = gama[38] = gama[39] = gama[40] = (int8_t)(2 * u);
		gama[41] = gama[42] = gama[43] = gama[44] = (int8_t)(-1 * u);
		gama[45] = gama[46] = gama[47] = gama[48] = (int8_t)(-1 * u);
		gama[49] = (int8_t)(-1 * u);
		gama[50] = gama[51] = gama[52] = gama[53] = (int8_t)(-4 * u);
		gama[54] = gama[55] = gama[56] = gama[57] = (int8_t)(-4 * u);
		gama[58] = gama[59] = gama[60] = (int8_t)(-7 * u);
		gama[61] = gama[62] = gama[63] = (int8_t)(-7 * u);
	}
}