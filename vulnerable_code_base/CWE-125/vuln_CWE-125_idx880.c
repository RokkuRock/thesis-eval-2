void luaT_adjustvarargs (lua_State *L, int nfixparams, CallInfo *ci,
                         const Proto *p) {
  int i;
  int actual = cast_int(L->top - ci->func) - 1;   
  int nextra = actual - nfixparams;   
  ci->u.l.nextraargs = nextra;
  checkstackGC(L, p->maxstacksize + 1);
  setobjs2s(L, L->top++, ci->func);
  for (i = 1; i <= nfixparams; i++) {
    setobjs2s(L, L->top++, ci->func + i);
    setnilvalue(s2v(ci->func + i));   
  }
  ci->func += actual + 1;
  ci->top += actual + 1;
  lua_assert(L->top <= ci->top && ci->top <= L->stack_last);
}