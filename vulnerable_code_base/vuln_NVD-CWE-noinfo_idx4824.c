static void dhcps_initialize_message(struct dhcp_msg *dhcp_message_repository)
{
        dhcp_message_repository->op = DHCP_MESSAGE_OP_REPLY;
        dhcp_message_repository->htype = DHCP_MESSAGE_HTYPE;
        dhcp_message_repository->hlen = DHCP_MESSAGE_HLEN; 
        dhcp_message_repository->hops = 0;		
        memcpy((char *)dhcp_recorded_xid, (char *) dhcp_message_repository->xid,
					sizeof(dhcp_message_repository->xid));
        dhcp_message_repository->secs = 0;
        dhcp_message_repository->flags = htons(BOOTP_BROADCAST);         
	memcpy((char *)dhcp_message_repository->yiaddr,
			(char *)&dhcps_allocated_client_address,
				sizeof(dhcp_message_repository->yiaddr));
	memset((char *)dhcp_message_repository->ciaddr, 0,
					sizeof(dhcp_message_repository->ciaddr));
        memset((char *)dhcp_message_repository->siaddr, 0,
					sizeof(dhcp_message_repository->siaddr));
        memset((char *)dhcp_message_repository->giaddr, 0,
					sizeof(dhcp_message_repository->giaddr));
        memset((char *)dhcp_message_repository->sname,  0,
					sizeof(dhcp_message_repository->sname));
        memset((char *)dhcp_message_repository->file,   0,
					sizeof(dhcp_message_repository->file));
        memset((char *)dhcp_message_repository->options, 0,
					dhcp_message_total_options_lenth);
        memcpy((char *)dhcp_message_repository->options, (char *)dhcp_magic_cookie,
					sizeof(dhcp_magic_cookie));
}