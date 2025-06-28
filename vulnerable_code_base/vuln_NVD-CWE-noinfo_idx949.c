static void mark_ip_in_table(uint8_t d)
{
#if (debug_dhcps)   
  	printf("\r\nmark ip %d\r\n",d);
#endif	
	xSemaphoreTake(dhcps_ip_table_semaphore, portMAX_DELAY);
	if (0 < d && d <= 32) {
		ip_table.ip_range[0] = MARK_RANGE1_IP_BIT(ip_table, d);	
#if (debug_dhcps)		
		printf("\r\n ip_table.ip_range[0] = 0x%x\r\n",ip_table.ip_range[0]);
#endif	
	} else if (32 < d && d <= 64) {
	  	ip_table.ip_range[1] = MARK_RANGE2_IP_BIT(ip_table, (d - 32));
#if (debug_dhcps)	
		printf("\r\n ip_table.ip_range[1] = 0x%x\r\n",ip_table.ip_range[1]);
#endif	
	} else if (64 < d && d <= 96) {
		ip_table.ip_range[2] = MARK_RANGE3_IP_BIT(ip_table, (d - 64));
#if (debug_dhcps)	
		printf("\r\n ip_table.ip_range[2] = 0x%x\r\n",ip_table.ip_range[2]);
#endif	
	} else if (96 < d && d <= 128) {
		ip_table.ip_range[3] = MARK_RANGE4_IP_BIT(ip_table, (d - 96));
#if (debug_dhcps)	
		printf("\r\n ip_table.ip_range[3] = 0x%x\r\n",ip_table.ip_range[3]);
#endif	
	} else if(128 < d && d <= 160) {
		ip_table.ip_range[4] = MARK_RANGE5_IP_BIT(ip_table, d);	
#if (debug_dhcps)		
		printf("\r\n ip_table.ip_range[4] = 0x%x\r\n",ip_table.ip_range[4]);
#endif	
	} else if (160 < d && d <= 192) {
		ip_table.ip_range[5] = MARK_RANGE6_IP_BIT(ip_table, (d - 160));
#if (debug_dhcps)	
		printf("\r\n ip_table.ip_range[5] = 0x%x\r\n",ip_table.ip_range[5]);
#endif	
	} else if (192 < d && d <= 224) {
		ip_table.ip_range[6] = MARK_RANGE7_IP_BIT(ip_table, (d - 192));
#if (debug_dhcps)	
		printf("\r\n ip_table.ip_range[6] = 0x%x\r\n",ip_table.ip_range[6]);
#endif	
	} else if (224 < d) {
		ip_table.ip_range[7] = MARK_RANGE8_IP_BIT(ip_table, (d - 224));
#if (debug_dhcps)	
		printf("\r\n ip_table.ip_range[7] = 0x%x\r\n",ip_table.ip_range[7]);
#endif	
	} else {
		printf("\r\n Request ip over the range(1-128) \r\n");
	}
	xSemaphoreGive(dhcps_ip_table_semaphore);
}