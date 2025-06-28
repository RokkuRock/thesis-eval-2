xmlAddID(xmlValidCtxtPtr ctxt, xmlDocPtr doc, const xmlChar *value,
         xmlAttrPtr attr) {
    xmlIDPtr ret;
    xmlIDTablePtr table;
    if (doc == NULL) {
	return(NULL);
    }
    if (value == NULL) {
	return(NULL);
    }
    if (attr == NULL) {
	return(NULL);
    }
    table = (xmlIDTablePtr) doc->ids;
    if (table == NULL)  {
        doc->ids = table = xmlHashCreateDict(0, doc->dict);
    }
    if (table == NULL) {
	xmlVErrMemory(ctxt,
		"xmlAddID: Table creation failed!\n");
        return(NULL);
    }
    ret = (xmlIDPtr) xmlMalloc(sizeof(xmlID));
    if (ret == NULL) {
	xmlVErrMemory(ctxt, "malloc failed");
	return(NULL);
    }
    ret->value = xmlStrdup(value);
    ret->doc = doc;
    if ((ctxt != NULL) && (ctxt->vstateNr != 0)) {
	if (doc->dict != NULL)
	    ret->name = xmlDictLookup(doc->dict, attr->name, -1);
	else
	    ret->name = xmlStrdup(attr->name);
	ret->attr = NULL;
    } else {
	ret->attr = attr;
	ret->name = NULL;
    }
    ret->lineno = xmlGetLineNo(attr->parent);
    if (xmlHashAddEntry(table, value, ret) < 0) {
#ifdef LIBXML_VALID_ENABLED
	if (ctxt != NULL) {
	    xmlErrValidNode(ctxt, attr->parent, XML_DTD_ID_REDEFINED,
			    "ID %s already defined\n", value, NULL, NULL);
	}
#endif  
	xmlFreeID(ret);
	return(NULL);
    }
    if (attr != NULL)
	attr->atype = XML_ATTRIBUTE_ID;
    return(ret);
}