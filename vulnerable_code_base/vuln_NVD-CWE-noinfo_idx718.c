uint8_t dhcps_handle_state_machine_change(uint8_t option_message_type)
{
	switch (option_message_type) {
	case DHCP_MESSAGE_TYPE_DECLINE:
		#if (debug_dhcps)	
		printf("\r\nget message DHCP_MESSAGE_TYPE_DECLINE\n");
		#endif
		dhcp_server_state_machine = DHCP_SERVER_STATE_IDLE;
		break;
	case DHCP_MESSAGE_TYPE_DISCOVER:
		#if (debug_dhcps)	
		printf("\r\nget message DHCP_MESSAGE_TYPE_DISCOVER\n");
		#endif
		if (dhcp_server_state_machine == DHCP_SERVER_STATE_IDLE) {
			dhcp_server_state_machine = DHCP_SERVER_STATE_OFFER;
		}
		break;
	case DHCP_MESSAGE_TYPE_REQUEST:
		#if (debug_dhcps)	
		printf("\r\n[%d]get message DHCP_MESSAGE_TYPE_REQUEST\n", xTaskGetTickCount());
		#endif
#if (!IS_USE_FIXED_IP) 	
#if (debug_dhcps)
		printf("\r\ndhcp_server_state_machine=%d", dhcp_server_state_machine);
		printf("\r\ndhcps_allocated_client_address=%d.%d.%d.%d", 
				ip4_addr1(&dhcps_allocated_client_address),
				ip4_addr2(&dhcps_allocated_client_address),
				ip4_addr3(&dhcps_allocated_client_address),
				ip4_addr4(&dhcps_allocated_client_address));
		printf("\r\nclient_request_ip=%d.%d.%d.%d\n", 
				ip4_addr1(&client_request_ip),
				ip4_addr2(&client_request_ip),
				ip4_addr3(&client_request_ip),
				ip4_addr4(&client_request_ip));
#endif		
		if (dhcp_server_state_machine == DHCP_SERVER_STATE_OFFER) {
			if (ip4_addr4(&dhcps_allocated_client_address) != 0) { 
				if (memcmp((void *)&dhcps_allocated_client_address, (void *)&client_request_ip, 4) == 0) {  	
					dhcp_server_state_machine = DHCP_SERVER_STATE_ACK;
			  	} else {
				  	dhcp_server_state_machine = DHCP_SERVER_STATE_NAK;
			  	}
			} else {
			  	dhcp_server_state_machine = DHCP_SERVER_STATE_NAK;
			}  
		} else if(dhcp_server_state_machine == DHCP_SERVER_STATE_IDLE){
			uint8_t ip_addr4 = check_client_request_ip(&client_request_ip, client_addr);
			if(ip_addr4 > 0){
				IP4_ADDR(&dhcps_allocated_client_address, (ip4_addr1(&dhcps_network_id)),
						ip4_addr2(&dhcps_network_id), ip4_addr3(&dhcps_network_id), ip_addr4);
				dhcp_server_state_machine = DHCP_SERVER_STATE_ACK;
			}else{
				dhcp_server_state_machine = DHCP_SERVER_STATE_NAK;
			}
		} else {
			dhcp_server_state_machine = DHCP_SERVER_STATE_NAK;
		}
#else		
		if (!(dhcp_server_state_machine == DHCP_SERVER_STATE_ACK ||
			dhcp_server_state_machine == DHCP_SERVER_STATE_NAK)) {
		        dhcp_server_state_machine = DHCP_SERVER_STATE_NAK;
		}
#endif
		break;
	case DHCP_MESSAGE_TYPE_RELEASE:
		printf("get message DHCP_MESSAGE_TYPE_RELEASE\n");
		dhcp_server_state_machine = DHCP_SERVER_STATE_IDLE;
		break;
	}
	return dhcp_server_state_machine;
}