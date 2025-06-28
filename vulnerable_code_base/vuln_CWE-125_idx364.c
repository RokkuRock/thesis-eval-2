void luaD_shrinkstack (lua_State *L) {
  int inuse = stackinuse(L);
  int goodsize = inuse + (inuse / 8) + 2*EXTRA_STACK;
  if (goodsize > LUAI_MAXSTACK)
    goodsize = LUAI_MAXSTACK;   
  if (inuse <= (LUAI_MAXSTACK - EXTRA_STACK) &&
      goodsize < L->stacksize)
    luaD_reallocstack(L, goodsize, 0);   
  else   
    condmovestack(L,{},{});   
  luaE_shrinkCI(L);   
}