static int edit_ext(char* editor, char* name, char* date, char* data)
{
	int fd;
	int st;
	int sz;
	char* b;
	char* l;
	char buff[512];
	pid_t pid;
	strcpy(buff,"/tmp/nodau.XXXXXX");
	fd = mkstemp(buff);
	if (fd < 0)
		return 1;
	pid = fork();
	if (pid < 0) {
		return 1;
	}else if (pid) {
		close(fd);
		waitpid(pid,&st,0);
		if (!st) {
			if ((fd = open(buff,O_RDONLY)) < 0)
				return 1;
			sz = lseek(fd,0,SEEK_END);
			lseek(fd,0,SEEK_SET);
			if (sz) {
				b = alloca(sz+1);
				if (sz != read(fd,b,sz))
					return 1;
				close(fd);
				remove(buff);
				b[sz] = 0;
				l = strstr(b,"-----");
				if (l) {
					l += 6;
					if (db_update(name,l))
						return 1;
					printf("%s saved\n",name);
				}
			}
		}
		return st;
	}
	sz = strlen(name)+strlen(date)+strlen(data)+50;
	b = alloca(sz);
	sz = sprintf(
		b,
		"%s (%s)\nText above this line is ignored\n-----\n%s",
		name,
		date,
		data
	);
	if (write(fd,b,sz) != sz) {
		exit(1);
	}
	fsync(fd);
	close(fd);
	st = execl(editor,editor,buff,(char*)NULL);
	exit(st);
	return 1;
}