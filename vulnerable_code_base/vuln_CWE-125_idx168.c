l2tp_result_code_print(netdissect_options *ndo, const u_char *dat, u_int length)
{
	const uint16_t *ptr = (const uint16_t *)dat;
	ND_PRINT((ndo, "%u", EXTRACT_16BITS(ptr))); ptr++;	 
	if (length > 2) {				 
	        ND_PRINT((ndo, "/%u", EXTRACT_16BITS(ptr))); ptr++;
	}
	if (length > 4) {				 
		ND_PRINT((ndo, " "));
		print_string(ndo, (const u_char *)ptr, length - 4);
	}
}