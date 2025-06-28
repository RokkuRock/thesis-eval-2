int luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                    int narg1, int delta) {
 retry:
  switch (ttypetag(s2v(func))) {
    case LUA_VCCL:   
      return precallC(L, func, LUA_MULTRET, clCvalue(s2v(func))->f);
    case LUA_VLCF:   
      return precallC(L, func, LUA_MULTRET, fvalue(s2v(func)));
    case LUA_VLCL: {   
      Proto *p = clLvalue(s2v(func))->p;
      int fsize = p->maxstacksize;   
      int nfixparams = p->numparams;
      int i;
      ci->func -= delta;   
      for (i = 0; i < narg1; i++)   
        setobjs2s(L, ci->func + i, func + i);
      checkstackGC(L, fsize);
      func = ci->func;   
      for (; narg1 <= nfixparams; narg1++)
        setnilvalue(s2v(func + narg1));   
      ci->top = func + 1 + fsize;   
      lua_assert(ci->top <= L->stack_last);
      ci->u.l.savedpc = p->code;   
      ci->callstatus |= CIST_TAIL;
      L->top = func + narg1;   
      return -1;
    }
    default: {   
      func = luaD_tryfuncTM(L, func);   
      narg1++;
      goto retry;   
    }
  }
}