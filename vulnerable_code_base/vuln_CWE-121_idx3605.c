Bool gf_sg_proto_field_is_sftime_offset(GF_Node *node, GF_FieldInfo *field)
{
	u32 i;
	GF_Route *r;
	GF_ProtoInstance *inst;
	GF_FieldInfo inf;
	if (node->sgprivate->tag != TAG_ProtoNode) return 0;
	if (field->fieldType != GF_SG_VRML_SFTIME) return 0;
	inst = (GF_ProtoInstance *) node;
	i=0;
	while ((r = (GF_Route*)gf_list_enum(inst->proto_interface->sub_graph->Routes, &i))) {
		if (!r->IS_route) continue;
		if (r->FromNode || (r->FromField.fieldIndex != field->fieldIndex)) continue;
		gf_node_get_field(r->ToNode, r->ToField.fieldIndex, &inf);
		if (r->ToNode->sgprivate->tag == TAG_ProtoNode) return gf_sg_proto_field_is_sftime_offset(r->ToNode, &inf);
		if (!stricmp(inf.name, "startTime") || !stricmp(inf.name, "stopTime")) return 1;
	}
	return 0;
}