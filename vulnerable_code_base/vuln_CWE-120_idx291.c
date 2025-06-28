int init_result(RESULT & result, void*& data) {
	FILE* f;
	std::string line;
	int retval, n;
	DATA* dp = new DATA;
	OUTPUT_FILE_INFO fi;
	log_messages.printf(MSG_DEBUG, "Start\n");
	retval = get_output_file_path(result, fi.path);
	if (retval) {
		log_messages.printf(MSG_CRITICAL, "Unable to open file\n");
		return -1;
	}
	f = fopen(fi.path.c_str(), "r");
	if (f == NULL) {
		log_messages.printf(MSG_CRITICAL,
				"Open error: %s\n errno: %s Waiting...\n", fi.path.c_str(),
				errno);
		usleep(1000);
		log_messages.printf(MSG_CRITICAL, "Try again...\n");
		f = fopen(fi.path.c_str(), "r");
		if (f == NULL) {
			return -1;
		}
	}
	log_messages.printf(MSG_DEBUG, "Check result\n");
	char buff[256];
	n = fscanf(f, "%s", buff);
	char * pch;
	pch = strtok(buff, " ,");
	if (pch != NULL) {
		dp->receptor = pch;
	} else {
		log_messages.printf(MSG_CRITICAL, "Seek receptor failed\n");
		return -1;
	}
	pch = strtok(NULL, ",");
	if (pch != NULL) {
		dp->ligand = pch;
	} else {
		log_messages.printf(MSG_CRITICAL, "Seek ligand failed\n");
		return -1;
	}
	pch = strtok(NULL, ",");
	if (pch != NULL) {
		dp->seed = strtod(pch, NULL);
	} else {
		log_messages.printf(MSG_CRITICAL, "Seek seed failed\n");
		return -1;
	}
	pch = strtok(NULL, ",");
	if (pch != NULL) {
		dp->score = atof(pch);
	} else {
		log_messages.printf(MSG_CRITICAL, "Seek score failed\n");
		return -1;
	}
	log_messages.printf(MSG_DEBUG, "%s %s %f %f\n", dp->receptor, dp->ligand,
			dp->seed, dp->score);
	if (strlen(dp->ligand) < 4 || strlen(dp->receptor) < 4) {
		log_messages.printf(MSG_CRITICAL, "%s %s Name failed\n", dp->receptor,
				dp->ligand);
		return -1;
	}
	data = (void*) dp;
	fclose(f);
	return 0;
}