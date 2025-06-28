mrb_proc_copy(mrb_state *mrb, struct RProc *a, struct RProc *b)
{
  if (a->body.irep) {
    return;
  }
  a->flags = b->flags;
  a->body = b->body;
  a->upper = b->upper;
  if (!MRB_PROC_CFUNC_P(a) && a->body.irep) {
    mrb_irep_incref(mrb, (mrb_irep*)a->body.irep);
  }
  a->e.env = b->e.env;
}