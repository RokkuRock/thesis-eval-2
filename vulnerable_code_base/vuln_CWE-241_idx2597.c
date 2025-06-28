parse_memory(VALUE klass, VALUE data)
{
  xmlParserCtxtPtr ctxt;
  if (NIL_P(data)) {
    rb_raise(rb_eArgError, "data cannot be nil");
  }
  if (!(int)RSTRING_LEN(data)) {
    rb_raise(rb_eRuntimeError, "data cannot be empty");
  }
  ctxt = xmlCreateMemoryParserCtxt(StringValuePtr(data),
                                   (int)RSTRING_LEN(data));
  if (ctxt->sax) {
    xmlFree(ctxt->sax);
    ctxt->sax = NULL;
  }
  return Data_Wrap_Struct(klass, NULL, deallocate, ctxt);
}