void luaV_concat (lua_State *L, int total) {
  if (total == 1)
    return;   
  do {
    StkId top = L->top;
    int n = 2;   
    if (!(ttisstring(s2v(top - 2)) || cvt2str(s2v(top - 2))) ||
        !tostring(L, s2v(top - 1)))
      luaT_tryconcatTM(L);
    else if (isemptystr(s2v(top - 1)))   
      cast_void(tostring(L, s2v(top - 2)));   
    else if (isemptystr(s2v(top - 2))) {   
      setobjs2s(L, top - 2, top - 1);   
    }
    else {
      size_t tl = vslen(s2v(top - 1));
      TString *ts;
      for (n = 1; n < total && tostring(L, s2v(top - n - 1)); n++) {
        size_t l = vslen(s2v(top - n - 1));
        if (l_unlikely(l >= (MAX_SIZE/sizeof(char)) - tl))
          luaG_runerror(L, "string length overflow");
        tl += l;
      }
      if (tl <= LUAI_MAXSHORTLEN) {   
        char buff[LUAI_MAXSHORTLEN];
        copy2buff(top, n, buff);   
        ts = luaS_newlstr(L, buff, tl);
      }
      else {   
        ts = luaS_createlngstrobj(L, tl);
        copy2buff(top, n, getstr(ts));
      }
      setsvalue2s(L, top - n, ts);   
    }
    total -= n-1;   
    L->top -= n-1;   
  } while (total > 1);   
}