int ieee80211_radiotap_iterator_init(
	struct ieee80211_radiotap_iterator *iterator,
	struct ieee80211_radiotap_header *radiotap_header,
	int max_length, const struct ieee80211_radiotap_vendor_namespaces *vns)
{
	if (radiotap_header->it_version)
		return -EINVAL;
	if (max_length < get_unaligned_le16(&radiotap_header->it_len))
		return -EINVAL;
	iterator->_rtheader = radiotap_header;
	iterator->_max_length = get_unaligned_le16(&radiotap_header->it_len);
	iterator->_arg_index = 0;
	iterator->_bitmap_shifter = get_unaligned_le32(&radiotap_header->it_present);
	iterator->_arg = (uint8_t *)radiotap_header + sizeof(*radiotap_header);
	iterator->_reset_on_ext = 0;
	iterator->_next_bitmap = &radiotap_header->it_present;
	iterator->_next_bitmap++;
	iterator->_vns = vns;
	iterator->current_namespace = &radiotap_ns;
	iterator->is_radiotap_ns = 1;
	if (iterator->_bitmap_shifter & (1<<IEEE80211_RADIOTAP_EXT)) {
		while (get_unaligned_le32(iterator->_arg) &
					(1 << IEEE80211_RADIOTAP_EXT)) {
			iterator->_arg += sizeof(uint32_t);
			if ((unsigned long)iterator->_arg -
			    (unsigned long)iterator->_rtheader >
			    (unsigned long)iterator->_max_length)
				return -EINVAL;
		}
		iterator->_arg += sizeof(uint32_t);
	}
	iterator->this_arg = iterator->_arg;
	return 0;
}