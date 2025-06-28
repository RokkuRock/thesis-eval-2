void luaT_getvarargs (lua_State *L, CallInfo *ci, StkId where, int wanted) {
  int i;
  int nextra = ci->u.l.nextraargs;
  if (wanted < 0) {
    wanted = nextra;   
    checkstackp(L, nextra, where);   
    L->top = where + nextra;   
  }
  for (i = 0; i < wanted && i < nextra; i++)
    setobjs2s(L, where + i, ci->func - nextra + i);
  for (; i < wanted; i++)    
    setnilvalue(s2v(where + i));
}