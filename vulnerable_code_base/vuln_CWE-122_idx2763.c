void gf_isom_check_position(GF_Box *s, GF_Box *child, u32 *pos)
{
	if (!s || !s->child_boxes || !child || !pos) return;
	if (s->internal_flags & GF_ISOM_ORDER_FREEZE)
		return;
	s32 cur_pos = gf_list_find(s->child_boxes, child);
	if (cur_pos < 0) return;
	if (cur_pos != (s32) *pos) {
		gf_list_del_item(s->child_boxes, child);
		gf_list_insert(s->child_boxes, child, *pos);
	}
	(*pos)++;