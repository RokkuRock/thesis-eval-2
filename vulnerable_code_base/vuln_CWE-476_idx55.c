void sqlcipher_exportFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  sqlite3 *db = sqlite3_context_db_handle(context);
  const char* targetDb, *sourceDb; 
  int targetDb_idx = 0;
  u64 saved_flags = db->flags;         
  u32 saved_mDbFlags = db->mDbFlags;         
  int saved_nChange = db->nChange;       
  int saved_nTotalChange = db->nTotalChange;  
  u8 saved_mTrace = db->mTrace;         
  int rc = SQLITE_OK;      
  char *zSql = NULL;          
  char *pzErrMsg = NULL;
  if(argc != 1 && argc != 2) {
    rc = SQLITE_ERROR;
    pzErrMsg = sqlite3_mprintf("invalid number of arguments (%d) passed to sqlcipher_export", argc);
    goto end_of_export;
  }
  targetDb = (const char*) sqlite3_value_text(argv[0]);
  sourceDb = (argc == 2) ? (char *) sqlite3_value_text(argv[1]) : "main";
  targetDb_idx =  sqlcipher_find_db_index(db, targetDb);
  if(targetDb_idx == 0 && sqlite3StrICmp("main", targetDb) != 0) {
    rc = SQLITE_ERROR;
    pzErrMsg = sqlite3_mprintf("unknown database %s", targetDb);
    goto end_of_export;
  }
  db->init.iDb = targetDb_idx;
  db->flags |= SQLITE_WriteSchema | SQLITE_IgnoreChecks; 
  db->mDbFlags |= DBFLAG_PreferBuiltin | DBFLAG_Vacuum;
  db->flags &= ~(u64)(SQLITE_ForeignKeys | SQLITE_ReverseOrder | SQLITE_Defensive | SQLITE_CountRows); 
  db->mTrace = 0;
  zSql = sqlite3_mprintf(
    "SELECT sql "
    "  FROM %s.sqlite_master WHERE type='table' AND name!='sqlite_sequence'"
    "   AND rootpage>0"
  , sourceDb);
  rc = (zSql == NULL) ? SQLITE_NOMEM : sqlcipher_execExecSql(db, &pzErrMsg, zSql); 
  if( rc!=SQLITE_OK ) goto end_of_export;
  sqlite3_free(zSql);
  zSql = sqlite3_mprintf(
    "SELECT sql "
    "  FROM %s.sqlite_master WHERE sql LIKE 'CREATE INDEX %%' "
  , sourceDb);
  rc = (zSql == NULL) ? SQLITE_NOMEM : sqlcipher_execExecSql(db, &pzErrMsg, zSql); 
  if( rc!=SQLITE_OK ) goto end_of_export;
  sqlite3_free(zSql);
  zSql = sqlite3_mprintf(
    "SELECT sql "
    "  FROM %s.sqlite_master WHERE sql LIKE 'CREATE UNIQUE INDEX %%'"
  , sourceDb);
  rc = (zSql == NULL) ? SQLITE_NOMEM : sqlcipher_execExecSql(db, &pzErrMsg, zSql); 
  if( rc!=SQLITE_OK ) goto end_of_export;
  sqlite3_free(zSql);
  zSql = sqlite3_mprintf(
    "SELECT 'INSERT INTO %s.' || quote(name) "
    "|| ' SELECT * FROM %s.' || quote(name) || ';'"
    "FROM %s.sqlite_master "
    "WHERE type = 'table' AND name!='sqlite_sequence' "
    "  AND rootpage>0"
  , targetDb, sourceDb, sourceDb);
  rc = (zSql == NULL) ? SQLITE_NOMEM : sqlcipher_execExecSql(db, &pzErrMsg, zSql); 
  if( rc!=SQLITE_OK ) goto end_of_export;
  sqlite3_free(zSql);
  zSql = sqlite3_mprintf(
    "SELECT 'INSERT INTO %s.' || quote(name) "
    "|| ' SELECT * FROM %s.' || quote(name) || ';' "
    "FROM %s.sqlite_master WHERE name=='sqlite_sequence';"
  , targetDb, sourceDb, targetDb);
  rc = (zSql == NULL) ? SQLITE_NOMEM : sqlcipher_execExecSql(db, &pzErrMsg, zSql); 
  if( rc!=SQLITE_OK ) goto end_of_export;
  sqlite3_free(zSql);
  zSql = sqlite3_mprintf(
    "INSERT INTO %s.sqlite_master "
    "  SELECT type, name, tbl_name, rootpage, sql"
    "    FROM %s.sqlite_master"
    "   WHERE type='view' OR type='trigger'"
    "      OR (type='table' AND rootpage=0)"
  , targetDb, sourceDb);
  rc = (zSql == NULL) ? SQLITE_NOMEM : sqlcipher_execSql(db, &pzErrMsg, zSql); 
  if( rc!=SQLITE_OK ) goto end_of_export;
  sqlite3_free(zSql);
  zSql = NULL;
end_of_export:
  db->init.iDb = 0;
  db->flags = saved_flags;
  db->mDbFlags = saved_mDbFlags;
  db->nChange = saved_nChange;
  db->nTotalChange = saved_nTotalChange;
  db->mTrace = saved_mTrace;
  if(zSql) sqlite3_free(zSql);
  if(rc) {
    if(pzErrMsg != NULL) {
      sqlite3_result_error(context, pzErrMsg, -1);
      sqlite3DbFree(db, pzErrMsg);
    } else {
      sqlite3_result_error(context, sqlite3ErrStr(rc), -1);
    }
  }
}