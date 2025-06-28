static void add_offer_options(uint8_t *option_start_address)
{
	uint8_t *temp_option_addr;
	temp_option_addr = fill_one_option_content(option_start_address,
			DHCP_OPTION_CODE_SUBNET_MASK, DHCP_OPTION_LENGTH_FOUR,
					(void *)&dhcps_local_mask);
	temp_option_addr = fill_one_option_content(temp_option_addr,
			DHCP_OPTION_CODE_ROUTER, DHCP_OPTION_LENGTH_FOUR,
					(void *)&dhcps_local_address);
	temp_option_addr = fill_one_option_content(temp_option_addr,
			DHCP_OPTION_CODE_DNS_SERVER, DHCP_OPTION_LENGTH_FOUR,
					(void *)&dhcps_local_address);	
	temp_option_addr = fill_one_option_content(temp_option_addr,
			DHCP_OPTION_CODE_LEASE_TIME, DHCP_OPTION_LENGTH_FOUR,
					(void *)&dhcp_option_lease_time);
	temp_option_addr = fill_one_option_content(temp_option_addr,
			DHCP_OPTION_CODE_SERVER_ID, DHCP_OPTION_LENGTH_FOUR,
				(void *)&dhcps_local_address);
	temp_option_addr = fill_one_option_content(temp_option_addr,
		DHCP_OPTION_CODE_BROADCAST_ADDRESS, DHCP_OPTION_LENGTH_FOUR,
				(void *)&dhcps_subnet_broadcast);
	temp_option_addr = fill_one_option_content(temp_option_addr,
		DHCP_OPTION_CODE_INTERFACE_MTU, DHCP_OPTION_LENGTH_TWO,
					(void *) &dhcp_option_interface_mtu); 
	temp_option_addr = fill_one_option_content(temp_option_addr,
		DHCP_OPTION_CODE_PERFORM_ROUTER_DISCOVERY, DHCP_OPTION_LENGTH_ONE,
								NULL);
	*temp_option_addr++ = DHCP_OPTION_CODE_END;
}