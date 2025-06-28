static void dhcps_receive_udp_packet_handler(void *arg, struct udp_pcb *udp_pcb,
struct pbuf *udp_packet_buffer, struct ip_addr *sender_addr, uint16_t sender_port)
{	
  	int16_t total_length_of_packet_buffer;
	struct pbuf *merged_packet_buffer = NULL;
	dhcp_message_repository = (struct dhcp_msg *)udp_packet_buffer->payload;
	if (udp_packet_buffer == NULL) {
		printf("\n\r Error!!!! System doesn't allocate any buffer \n\r");
		return;  
	}
	if (sender_port == DHCP_CLIENT_PORT) {
		total_length_of_packet_buffer = udp_packet_buffer->tot_len;
		if (udp_packet_buffer->next != NULL) {
			merged_packet_buffer = pbuf_coalesce(udp_packet_buffer,
								PBUF_TRANSPORT);
			if (merged_packet_buffer->tot_len !=
						total_length_of_packet_buffer) {
				pbuf_free(udp_packet_buffer);	
				return;
			}
		}
		switch (dhcps_check_msg_and_handle_options(udp_packet_buffer)) {
		case  DHCP_SERVER_STATE_OFFER:
			#if (debug_dhcps)	
			printf("%s DHCP_SERVER_STATE_OFFER\n",__func__);
			#endif
			dhcps_send_offer(udp_packet_buffer);
			break;
		case DHCP_SERVER_STATE_ACK:
			#if (debug_dhcps)	
			printf("%s DHCP_SERVER_STATE_ACK\n",__func__);
			#endif
			dhcps_send_ack(udp_packet_buffer);
#if (!IS_USE_FIXED_IP)
			mark_ip_in_table((uint8_t)ip4_addr4(&dhcps_allocated_client_address)); 			
	#ifdef CONFIG_DHCPS_KEPT_CLIENT_INFO
			save_client_addr(&dhcps_allocated_client_address, client_addr);
			memset(&client_request_ip, 0, sizeof(client_request_ip));
			memset(&client_addr, 0, sizeof(client_addr));
			memset(&dhcps_allocated_client_address, 0, sizeof(dhcps_allocated_client_address));
			#if (debug_dhcps)	
			dump_client_table();
			#endif
	#endif
#endif
			dhcp_server_state_machine = DHCP_SERVER_STATE_IDLE;
			break;
		case DHCP_SERVER_STATE_NAK:
			#if (debug_dhcps)	
			printf("%s DHCP_SERVER_STATE_NAK\n",__func__);
			#endif
			dhcps_send_nak(udp_packet_buffer);
			dhcp_server_state_machine = DHCP_SERVER_STATE_IDLE;
			break;
		case DHCP_OPTION_CODE_END:
			#if (debug_dhcps)	
			printf("%s DHCP_OPTION_CODE_END\n",__func__);
			#endif
			break;
		}
	}
	udp_disconnect(udp_pcb);
	if (merged_packet_buffer != NULL)
		pbuf_free(merged_packet_buffer);
	else 
		pbuf_free(udp_packet_buffer);
}