void dhcps_deinit(void)
{
	if (dhcps_pcb != NULL) {
		udp_remove(dhcps_pcb);
		dhcps_pcb = NULL;	
	}
	if (dhcps_ip_table_semaphore != NULL) {	
		vSemaphoreDelete(dhcps_ip_table_semaphore);
		dhcps_ip_table_semaphore = NULL;
	}		
}