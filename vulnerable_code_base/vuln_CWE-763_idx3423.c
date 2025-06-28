static GCObject **correctgraylist (GCObject **p) {
  GCObject *curr;
  while ((curr = *p) != NULL) {
    switch (curr->tt) {
      case LUA_VTABLE: case LUA_VUSERDATA: {
        GCObject **next = getgclist(curr);
        if (getage(curr) == G_TOUCHED1) {   
          lua_assert(isgray(curr));
          gray2black(curr);   
          changeage(curr, G_TOUCHED1, G_TOUCHED2);
          p = next;   
        }
        else {   
          if (!iswhite(curr)) {   
            lua_assert(isold(curr));
            if (getage(curr) == G_TOUCHED2)   
              changeage(curr, G_TOUCHED2, G_OLD);   
            gray2black(curr);   
          }
          *p = *next;   
        }
        break;
      }
      case LUA_VTHREAD: {
        lua_State *th = gco2th(curr);
        lua_assert(!isblack(th));
        if (iswhite(th))   
          *p = th->gclist;   
        else   
          p = &th->gclist;   
        break;
      }
      default: lua_assert(0);   
    }
  }
  return p;
}