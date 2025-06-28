internalEntityProcessor(XML_Parser parser, const char *s, const char *end,
                        const char **nextPtr) {
  ENTITY *entity;
  const char *textStart, *textEnd;
  const char *next;
  enum XML_Error result;
  OPEN_INTERNAL_ENTITY *openEntity = parser->m_openInternalEntities;
  if (! openEntity)
    return XML_ERROR_UNEXPECTED_STATE;
  entity = openEntity->entity;
  textStart = ((char *)entity->textPtr) + entity->processed;
  textEnd = (char *)(entity->textPtr + entity->textLen);
  next = textStart;
#ifdef XML_DTD
  if (entity->is_param) {
    int tok
        = XmlPrologTok(parser->m_internalEncoding, textStart, textEnd, &next);
    result = doProlog(parser, parser->m_internalEncoding, textStart, textEnd,
                      tok, next, &next, XML_FALSE);
  } else
#endif  
    result = doContent(parser, openEntity->startTagLevel,
                       parser->m_internalEncoding, textStart, textEnd, &next,
                       XML_FALSE);
  if (result != XML_ERROR_NONE)
    return result;
  else if (textEnd != next
           && parser->m_parsingStatus.parsing == XML_SUSPENDED) {
    entity->processed = (int)(next - (char *)entity->textPtr);
    return result;
  } else {
    entity->open = XML_FALSE;
    parser->m_openInternalEntities = openEntity->next;
    openEntity->next = parser->m_freeInternalEntities;
    parser->m_freeInternalEntities = openEntity;
  }
#ifdef XML_DTD
  if (entity->is_param) {
    int tok;
    parser->m_processor = prologProcessor;
    tok = XmlPrologTok(parser->m_encoding, s, end, &next);
    return doProlog(parser, parser->m_encoding, s, end, tok, next, nextPtr,
                    (XML_Bool)! parser->m_parsingStatus.finalBuffer);
  } else
#endif  
  {
    parser->m_processor = contentProcessor;
    return doContent(parser, parser->m_parentParser ? 1 : 0, parser->m_encoding,
                     s, end, nextPtr,
                     (XML_Bool)! parser->m_parsingStatus.finalBuffer);
  }
}