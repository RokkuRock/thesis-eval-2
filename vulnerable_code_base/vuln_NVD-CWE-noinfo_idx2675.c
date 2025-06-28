static void save_client_addr(struct ip_addr *client_ip, uint8_t *hwaddr)
{
	uint8_t d = (uint8_t)ip4_addr4(client_ip);
	xSemaphoreTake(dhcps_ip_table_semaphore, portMAX_DELAY);
	memcpy(ip_table.client_mac[d], hwaddr, 6); 
#if (debug_dhcps)	
	printf("\r\n%s: ip %d.%d.%d.%d, hwaddr %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n", __func__,
		ip4_addr1(client_ip), ip4_addr2(client_ip), ip4_addr3(client_ip), ip4_addr4(client_ip),
		hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
#endif	
	xSemaphoreGive(dhcps_ip_table_semaphore);
}