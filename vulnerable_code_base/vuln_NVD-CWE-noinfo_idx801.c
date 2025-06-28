static void dhcps_send_ack(struct pbuf *packet_buffer)
{
	dhcp_message_repository = (struct dhcp_msg *)packet_buffer->payload;
	dhcps_initialize_message(dhcp_message_repository);
	add_offer_options(add_msg_type(&dhcp_message_repository->options[4],
			      			DHCP_MESSAGE_TYPE_ACK));
	udp_sendto_if(dhcps_pcb, packet_buffer,
		   &dhcps_send_broadcast_address, DHCP_CLIENT_PORT, dhcps_netif);
}