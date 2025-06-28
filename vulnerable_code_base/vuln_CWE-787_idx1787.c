int dns_add_rr_nested_memcpy(struct dns_rr_nested *rr_nested, void *data, int data_len)
{
	if (rr_nested == NULL || data == NULL || data_len <= 0) {
		return -1;
	}
	if (_dns_left_len(&rr_nested->context) < data_len) {
		return -1;
	}
	memcpy(rr_nested->context.ptr, data, data_len);
	rr_nested->context.ptr += data_len;
	return 0;
}