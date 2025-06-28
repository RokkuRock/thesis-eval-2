_pdfioTokenGet(_pdfio_token_t *tb,	 
	       char           *buffer,	 
	       size_t         bufsize)	 
{
  if (tb->num_tokens > 0)
  {
    tb->num_tokens --;
    strncpy(buffer, tb->tokens[tb->num_tokens], bufsize - 1);
    buffer[bufsize - 1] = '\0';
    PDFIO_DEBUG("_pdfioTokenGet(tb=%p, buffer=%p, bufsize=%u): Popping '%s' from stack.\n", tb, buffer, (unsigned)bufsize, buffer);
    free(tb->tokens[tb->num_tokens]);
    tb->tokens[tb->num_tokens] = NULL;
    return (true);
  }
  return (_pdfioTokenRead(tb, buffer, bufsize));
}