show_tree(tree_t *t,                     
          int    indent)                 
{
  while (t)
  {
    if (t->markup == MARKUP_NONE)
      printf("%*s\"%s\"\n", indent, "", t->data);
    else
      printf("%*s%s\n", indent, "", _htmlMarkups[t->markup]);
    if (t->child)
      show_tree(t->child, indent + 2);
    t = t->next;
  }
}