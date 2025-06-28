int luaG_traceexec (lua_State *L, const Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte mask = L->hookmask;
  int counthook;
  if (!(mask & (LUA_MASKLINE | LUA_MASKCOUNT))) {   
    ci->u.l.trap = 0;   
    return 0;   
  }
  pc++;   
  ci->u.l.savedpc = pc;   
  counthook = (--L->hookcount == 0 && (mask & LUA_MASKCOUNT));
  if (counthook)
    resethookcount(L);   
  else if (!(mask & LUA_MASKLINE))
    return 1;   
  if (ci->callstatus & CIST_HOOKYIELD) {   
    ci->callstatus &= ~CIST_HOOKYIELD;   
    return 1;   
  }
  if (!isIT(*(ci->u.l.savedpc - 1)))
    L->top = ci->top;   
  if (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);   
  if (mask & LUA_MASKLINE) {
    const Proto *p = ci_func(ci)->p;
    int npci = pcRel(pc, p);
    if (npci == 0 ||   
        pc <= L->oldpc ||   
        changedline(p, pcRel(L->oldpc, p), npci)) {   
      int newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);   
    }
    L->oldpc = pc;   
  }
  if (L->status == LUA_YIELD) {   
    if (counthook)
      L->hookcount = 1;   
    ci->u.l.savedpc--;   
    ci->callstatus |= CIST_HOOKYIELD;   
    luaD_throw(L, LUA_YIELD);
  }
  return 1;   
}