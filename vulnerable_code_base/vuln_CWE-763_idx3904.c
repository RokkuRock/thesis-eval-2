void luaC_barrier_ (lua_State *L, GCObject *o, GCObject *v) {
  global_State *g = G(L);
  lua_assert(isblack(o) && iswhite(v) && !isdead(g, v) && !isdead(g, o));
  if (keepinvariant(g)) {   
    reallymarkobject(g, v);   
    if (isold(o)) {
      lua_assert(!isold(v));   
      setage(v, G_OLD0);   
    }
  }
  else {   
    lua_assert(issweepphase(g));
    makewhite(g, o);   
  }
}