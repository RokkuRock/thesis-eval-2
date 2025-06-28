const char *luaG_findlocal (lua_State *L, CallInfo *ci, int n, StkId *pos) {
  StkId base = ci->func + 1;
  const char *name = NULL;
  if (isLua(ci)) {
    if (n < 0)   
      return findvararg(ci, -n, pos);
    else
      name = luaF_getlocalname(ci_func(ci)->p, n, currentpc(ci));
  }
  if (name == NULL) {   
    StkId limit = (ci == L->ci) ? L->top : ci->next->func;
    if (limit - base >= n && n > 0) {   
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    else
      return NULL;   
  }
  if (pos)
    *pos = base + (n - 1);
  return name;
}