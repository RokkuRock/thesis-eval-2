mrb_proc_copy(struct RProc *a, struct RProc *b)
{
  if (a->body.irep) {
    return;
  }
  a->flags = b->flags;
  a->body = b->body;
  if (!MRB_PROC_CFUNC_P(a) && a->body.irep) {
    mrb_irep_incref(NULL, (mrb_irep*)a->body.irep);
  }
  a->upper = b->upper;
  a->e.env = b->e.env;
}