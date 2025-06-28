static void youngcollection (lua_State *L, global_State *g) {
  GCObject **psurvival;   
  lua_assert(g->gcstate == GCSpropagate);
  markold(g, g->survival, g->reallyold);
  markold(g, g->finobj, g->finobjrold);
  atomic(L);
  psurvival = sweepgen(L, g, &g->allgc, g->survival);
  sweepgen(L, g, psurvival, g->reallyold);
  g->reallyold = g->old;
  g->old = *psurvival;   
  g->survival = g->allgc;   
  psurvival = sweepgen(L, g, &g->finobj, g->finobjsur);
  sweepgen(L, g, psurvival, g->finobjrold);
  g->finobjrold = g->finobjold;
  g->finobjold = *psurvival;   
  g->finobjsur = g->finobj;   
  sweepgen(L, g, &g->tobefnz, NULL);
  finishgencycle(L, g);
}