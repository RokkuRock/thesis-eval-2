PLIST_API void plist_from_xml(const char *plist_xml, uint32_t length, plist_t * plist)
{
    xmlDocPtr plist_doc = xmlParseMemory(plist_xml, length);
    xmlNodePtr root_node = xmlDocGetRootElement(plist_doc);
    xml_to_node(root_node, plist);
    xmlFreeDoc(plist_doc);
}