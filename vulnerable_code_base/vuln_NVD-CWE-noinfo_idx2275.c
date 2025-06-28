static void dhcps_send_offer(struct pbuf *packet_buffer)
{
	uint8_t temp_ip = 0;
	dhcp_message_repository = (struct dhcp_msg *)packet_buffer->payload;	
#if (!IS_USE_FIXED_IP) 	
	temp_ip = check_client_request_ip(&client_request_ip, client_addr);
	if(temp_ip == 0)
		temp_ip = search_next_ip();
#if (debug_dhcps)	
	printf("\r\n temp_ip = %d",temp_ip);
#endif	
	if (temp_ip == 0) {
#if 0	
	  	memset(&ip_table, 0, sizeof(struct table));
		mark_ip_in_table((uint8_t)ip4_addr4(&dhcps_local_address));
		printf("\r\n reset ip table!!\r\n");	
#endif	
		printf("\r\n No useable ip!!!!\r\n");
	}
	printf("\n\r[%d]DHCP assign ip = %d.%d.%d.%d\n", xTaskGetTickCount(), ip4_addr1(&dhcps_network_id),ip4_addr2(&dhcps_network_id),ip4_addr3(&dhcps_network_id),temp_ip);
	IP4_ADDR(&dhcps_allocated_client_address, (ip4_addr1(&dhcps_network_id)),
			ip4_addr2(&dhcps_network_id), ip4_addr3(&dhcps_network_id), temp_ip);
#endif   
	dhcps_initialize_message(dhcp_message_repository);
	add_offer_options(add_msg_type(&dhcp_message_repository->options[4],
			DHCP_MESSAGE_TYPE_OFFER));
	udp_sendto_if(dhcps_pcb, packet_buffer,
			&dhcps_send_broadcast_address, DHCP_CLIENT_PORT, dhcps_netif);
}