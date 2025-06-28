xmlParsePEReference(xmlParserCtxtPtr ctxt)
{
    const xmlChar *name;
    xmlEntityPtr entity = NULL;
    xmlParserInputPtr input;
    if (RAW != '%')
        return;
    NEXT;
    name = xmlParseName(ctxt);
    if (name == NULL) {
	xmlFatalErrMsg(ctxt, XML_ERR_PEREF_NO_NAME, "PEReference: no name\n");
	return;
    }
    if (xmlParserDebugEntities)
	xmlGenericError(xmlGenericErrorContext,
		"PEReference: %s\n", name);
    if (RAW != ';') {
	xmlFatalErr(ctxt, XML_ERR_PEREF_SEMICOL_MISSING, NULL);
        return;
    }
    NEXT;
    ctxt->nbentities++;
    if ((ctxt->sax != NULL) &&
	(ctxt->sax->getParameterEntity != NULL))
	entity = ctxt->sax->getParameterEntity(ctxt->userData, name);
    if (ctxt->instate == XML_PARSER_EOF)
	return;
    if (entity == NULL) {
	if ((ctxt->standalone == 1) ||
	    ((ctxt->hasExternalSubset == 0) &&
	     (ctxt->hasPErefs == 0))) {
	    xmlFatalErrMsgStr(ctxt, XML_ERR_UNDECLARED_ENTITY,
			      "PEReference: %%%s; not found\n",
			      name);
	} else {
            if ((ctxt->validate) && (ctxt->vctxt.error != NULL)) {
                xmlValidityError(ctxt, XML_WAR_UNDECLARED_ENTITY,
                                 "PEReference: %%%s; not found\n",
                                 name, NULL);
            } else
                xmlWarningMsg(ctxt, XML_WAR_UNDECLARED_ENTITY,
                              "PEReference: %%%s; not found\n",
                              name, NULL);
            ctxt->valid = 0;
	}
	xmlParserEntityCheck(ctxt, 0, NULL, 0);
    } else {
	if ((entity->etype != XML_INTERNAL_PARAMETER_ENTITY) &&
	    (entity->etype != XML_EXTERNAL_PARAMETER_ENTITY)) {
	    xmlWarningMsg(ctxt, XML_WAR_UNDECLARED_ENTITY,
		  "Internal: %%%s; is not a parameter entity\n",
			  name, NULL);
	} else {
            xmlChar start[4];
            xmlCharEncoding enc;
	    if ((entity->etype == XML_EXTERNAL_PARAMETER_ENTITY) &&
	        ((ctxt->options & XML_PARSE_NOENT) == 0) &&
		((ctxt->options & XML_PARSE_DTDVALID) == 0) &&
		((ctxt->options & XML_PARSE_DTDLOAD) == 0) &&
		((ctxt->options & XML_PARSE_DTDATTR) == 0) &&
		(ctxt->replaceEntities == 0) &&
		(ctxt->validate == 0))
		return;
	    input = xmlNewEntityInputStream(ctxt, entity);
	    if (xmlPushInput(ctxt, input) < 0)
		return;
	    if (entity->etype == XML_EXTERNAL_PARAMETER_ENTITY) {
                GROW
                if (ctxt->instate == XML_PARSER_EOF)
                    return;
                if ((ctxt->input->end - ctxt->input->cur)>=4) {
                    start[0] = RAW;
                    start[1] = NXT(1);
                    start[2] = NXT(2);
                    start[3] = NXT(3);
                    enc = xmlDetectCharEncoding(start, 4);
                    if (enc != XML_CHAR_ENCODING_NONE) {
                        xmlSwitchEncoding(ctxt, enc);
                    }
                }
                if ((CMP5(CUR_PTR, '<', '?', 'x', 'm', 'l')) &&
                    (IS_BLANK_CH(NXT(5)))) {
                    xmlParseTextDecl(ctxt);
                }
            }
	}
    }
    ctxt->hasPErefs = 1;
}