int sqlite3CheckObjectName(
  Parse *pParse,             
  const char *zName,         
  const char *zType,         
  const char *zTblName       
){
  sqlite3 *db = pParse->db;
  if( sqlite3WritableSchema(db) || db->init.imposterTable ){
    return SQLITE_OK;
  }
  if( db->init.busy ){
    if( sqlite3_stricmp(zType, db->init.azInit[0])
     || sqlite3_stricmp(zName, db->init.azInit[1])
     || sqlite3_stricmp(zTblName, db->init.azInit[2])
    ){
      if( sqlite3Config.bExtraSchemaChecks ){
        sqlite3ErrorMsg(pParse, "");  
        return SQLITE_ERROR;
      }
    }
  }else{
    if( pParse->nested==0 
     && 0==sqlite3StrNICmp(zName, "sqlite_", 7)
    ){
      sqlite3ErrorMsg(pParse, "object name reserved for internal use: %s",
                      zName);
      return SQLITE_ERROR;
    }
  }
  return SQLITE_OK;
}