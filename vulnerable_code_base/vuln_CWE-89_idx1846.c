int db_update(char* name, char* value)
{
	char* sql;
	int r = 0;
	if (crypt_key) {
		value = note_encrypt(value,crypt_key);
		r = asprintf(&sql, "UPDATE nodau set text='%s' , encrypted='true' WHERE name='%s'", value, name);
		free(value);
		if (r < 0)
			return 1;
	}else{
		if (asprintf(&sql, "UPDATE nodau set text='%s' , encrypted='false' WHERE name='%s'", value, name) < 0)
			return 1;
	}
	r = sqlite3_exec(db_data.db, sql, NULL, 0, &db_data.error_msg);
	free(sql);
	return r;
}