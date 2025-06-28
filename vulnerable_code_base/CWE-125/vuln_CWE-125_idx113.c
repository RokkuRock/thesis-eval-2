static void parse_hid_report_descriptor(struct gtco *device, char * report,
					int length)
{
	struct device *ddev = &device->intf->dev;
	int   x, i = 0;
	__u8   prefix;
	__u8   size;
	__u8   tag;
	__u8   type;
	__u8   data   = 0;
	__u16  data16 = 0;
	__u32  data32 = 0;
	int   inputnum = 0;
	__u32 usage = 0;
	__u32 globalval[TAG_GLOB_MAX];
	__u32 oldval[TAG_GLOB_MAX];
	char  maintype = 'x';
	char  globtype[12];
	int   indent = 0;
	char  indentstr[10] = "";
	dev_dbg(ddev, "======>>>>>>PARSE<<<<<<======\n");
	while (i < length) {
		prefix = report[i];
		i++;
		size = PREF_SIZE(prefix);
		switch (size) {
		case 1:
			data = report[i];
			break;
		case 2:
			data16 = get_unaligned_le16(&report[i]);
			break;
		case 3:
			size = 4;
			data32 = get_unaligned_le32(&report[i]);
			break;
		}
		i += size;
		tag  = PREF_TAG(prefix);
		type = PREF_TYPE(prefix);
		switch (type) {
		case TYPE_MAIN:
			strcpy(globtype, "");
			switch (tag) {
			case TAG_MAIN_INPUT:
				maintype = 'I';
				if (data == 2)
					strcpy(globtype, "Variable");
				else if (data == 3)
					strcpy(globtype, "Var|Const");
				dev_dbg(ddev, "::::: Saving Report: %d input #%d Max: 0x%X(%d) Min:0x%X(%d) of %d bits\n",
					globalval[TAG_GLOB_REPORT_ID], inputnum,
					globalval[TAG_GLOB_LOG_MAX], globalval[TAG_GLOB_LOG_MAX],
					globalval[TAG_GLOB_LOG_MIN], globalval[TAG_GLOB_LOG_MIN],
					globalval[TAG_GLOB_REPORT_SZ] * globalval[TAG_GLOB_REPORT_CNT]);
				switch (inputnum) {
				case 0:   
					dev_dbg(ddev, "GER: X Usage: 0x%x\n", usage);
					if (device->max_X == 0) {
						device->max_X = globalval[TAG_GLOB_LOG_MAX];
						device->min_X = globalval[TAG_GLOB_LOG_MIN];
					}
					break;
				case 1:   
					dev_dbg(ddev, "GER: Y Usage: 0x%x\n", usage);
					if (device->max_Y == 0) {
						device->max_Y = globalval[TAG_GLOB_LOG_MAX];
						device->min_Y = globalval[TAG_GLOB_LOG_MIN];
					}
					break;
				default:
					if (usage == DIGITIZER_USAGE_TILT_X) {
						if (device->maxtilt_X == 0) {
							device->maxtilt_X = globalval[TAG_GLOB_LOG_MAX];
							device->mintilt_X = globalval[TAG_GLOB_LOG_MIN];
						}
					}
					if (usage == DIGITIZER_USAGE_TILT_Y) {
						if (device->maxtilt_Y == 0) {
							device->maxtilt_Y = globalval[TAG_GLOB_LOG_MAX];
							device->mintilt_Y = globalval[TAG_GLOB_LOG_MIN];
						}
					}
					if (usage == DIGITIZER_USAGE_TIP_PRESSURE) {
						if (device->maxpressure == 0) {
							device->maxpressure = globalval[TAG_GLOB_LOG_MAX];
							device->minpressure = globalval[TAG_GLOB_LOG_MIN];
						}
					}
					break;
				}
				inputnum++;
				break;
			case TAG_MAIN_OUTPUT:
				maintype = 'O';
				break;
			case TAG_MAIN_FEATURE:
				maintype = 'F';
				break;
			case TAG_MAIN_COL_START:
				maintype = 'S';
				if (data == 0) {
					dev_dbg(ddev, "======>>>>>> Physical\n");
					strcpy(globtype, "Physical");
				} else
					dev_dbg(ddev, "======>>>>>>\n");
				indent++;
				for (x = 0; x < indent; x++)
					indentstr[x] = '-';
				indentstr[x] = 0;
				for (x = 0; x < TAG_GLOB_MAX; x++)
					oldval[x] = globalval[x];
				break;
			case TAG_MAIN_COL_END:
				dev_dbg(ddev, "<<<<<<======\n");
				maintype = 'E';
				indent--;
				for (x = 0; x < indent; x++)
					indentstr[x] = '-';
				indentstr[x] = 0;
				for (x = 0; x < TAG_GLOB_MAX; x++)
					globalval[x] = oldval[x];
				break;
			}
			switch (size) {
			case 1:
				dev_dbg(ddev, "%sMAINTAG:(%d) %c SIZE: %d Data: %s 0x%x\n",
					indentstr, tag, maintype, size, globtype, data);
				break;
			case 2:
				dev_dbg(ddev, "%sMAINTAG:(%d) %c SIZE: %d Data: %s 0x%x\n",
					indentstr, tag, maintype, size, globtype, data16);
				break;
			case 4:
				dev_dbg(ddev, "%sMAINTAG:(%d) %c SIZE: %d Data: %s 0x%x\n",
					indentstr, tag, maintype, size, globtype, data32);
				break;
			}
			break;
		case TYPE_GLOBAL:
			switch (tag) {
			case TAG_GLOB_USAGE:
				if (device->usage == 0)
					device->usage = data;
				strcpy(globtype, "USAGE");
				break;
			case TAG_GLOB_LOG_MIN:
				strcpy(globtype, "LOG_MIN");
				break;
			case TAG_GLOB_LOG_MAX:
				strcpy(globtype, "LOG_MAX");
				break;
			case TAG_GLOB_PHYS_MIN:
				strcpy(globtype, "PHYS_MIN");
				break;
			case TAG_GLOB_PHYS_MAX:
				strcpy(globtype, "PHYS_MAX");
				break;
			case TAG_GLOB_UNIT_EXP:
				strcpy(globtype, "EXP");
				break;
			case TAG_GLOB_UNIT:
				strcpy(globtype, "UNIT");
				break;
			case TAG_GLOB_REPORT_SZ:
				strcpy(globtype, "REPORT_SZ");
				break;
			case TAG_GLOB_REPORT_ID:
				strcpy(globtype, "REPORT_ID");
				inputnum = 0;
				break;
			case TAG_GLOB_REPORT_CNT:
				strcpy(globtype, "REPORT_CNT");
				break;
			case TAG_GLOB_PUSH:
				strcpy(globtype, "PUSH");
				break;
			case TAG_GLOB_POP:
				strcpy(globtype, "POP");
				break;
			}
			if (tag < TAG_GLOB_MAX) {
				switch (size) {
				case 1:
					dev_dbg(ddev, "%sGLOBALTAG:%s(%d) SIZE: %d Data: 0x%x\n",
						indentstr, globtype, tag, size, data);
					globalval[tag] = data;
					break;
				case 2:
					dev_dbg(ddev, "%sGLOBALTAG:%s(%d) SIZE: %d Data: 0x%x\n",
						indentstr, globtype, tag, size, data16);
					globalval[tag] = data16;
					break;
				case 4:
					dev_dbg(ddev, "%sGLOBALTAG:%s(%d) SIZE: %d Data: 0x%x\n",
						indentstr, globtype, tag, size, data32);
					globalval[tag] = data32;
					break;
				}
			} else {
				dev_dbg(ddev, "%sGLOBALTAG: ILLEGAL TAG:%d SIZE: %d\n",
					indentstr, tag, size);
			}
			break;
		case TYPE_LOCAL:
			switch (tag) {
			case TAG_GLOB_USAGE:
				strcpy(globtype, "USAGE");
				usage = data;
				break;
			case TAG_GLOB_LOG_MIN:
				strcpy(globtype, "MIN");
				break;
			case TAG_GLOB_LOG_MAX:
				strcpy(globtype, "MAX");
				break;
			default:
				strcpy(globtype, "UNKNOWN");
				break;
			}
			switch (size) {
			case 1:
				dev_dbg(ddev, "%sLOCALTAG:(%d) %s SIZE: %d Data: 0x%x\n",
					indentstr, tag, globtype, size, data);
				break;
			case 2:
				dev_dbg(ddev, "%sLOCALTAG:(%d) %s SIZE: %d Data: 0x%x\n",
					indentstr, tag, globtype, size, data16);
				break;
			case 4:
				dev_dbg(ddev, "%sLOCALTAG:(%d) %s SIZE: %d Data: 0x%x\n",
					indentstr, tag, globtype, size, data32);
				break;
			}
			break;
		}
	}
}