MakeFilename(char *buffer, char *orig_name, int cnt, int max_chars)
{
        char *filename = malloc(PATH_MAX + 1);
        if (filename == NULL)
            error("%s: malloc", __func__);
        if (Gflag != 0) {
          struct tm *local_tm;
          if ((local_tm = localtime(&Gflag_time)) == NULL) {
                  error("%s: localtime", __func__);
          }
          strftime(filename, PATH_MAX, orig_name, local_tm);
        } else {
          strncpy(filename, orig_name, PATH_MAX);
        }
	if (cnt == 0 && max_chars == 0)
		strncpy(buffer, filename, PATH_MAX + 1);
	else
		if (snprintf(buffer, PATH_MAX + 1, "%s%0*d", filename, max_chars, cnt) > PATH_MAX)
                  error("too many output files or filename is too long (> %d)", PATH_MAX);
        free(filename);
}