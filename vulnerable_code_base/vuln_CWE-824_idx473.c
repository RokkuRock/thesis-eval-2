static bool vtable_is_value_in_text_section(RVTableContext *context, ut64 curAddress, ut64 *value) {
	ut64 curAddressValue;
	if (!context->read_addr (context->anal, curAddress, &curAddressValue)) {
		return false;
	}
	bool ret = vtable_addr_in_text_section (context, curAddressValue);
	if (value) {
		*value = curAddressValue;
	}
	return ret;
}