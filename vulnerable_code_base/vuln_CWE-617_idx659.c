cdp_decode(struct lldpd *cfg, char *frame, int s,
    struct lldpd_hardware *hardware,
    struct lldpd_chassis **newchassis, struct lldpd_port **newport)
{
	struct lldpd_chassis *chassis;
	struct lldpd_port *port;
	struct lldpd_mgmt *mgmt;
	struct in_addr addr;
#if 0
	u_int16_t cksum;
#endif
	u_int8_t *software = NULL, *platform = NULL;
	int software_len = 0, platform_len = 0, proto, version, nb, caps;
	const unsigned char cdpaddr[] = CDP_MULTICAST_ADDR;
#ifdef ENABLE_FDP
	const unsigned char fdpaddr[] = CDP_MULTICAST_ADDR;
	int fdp = 0;
#endif
	u_int8_t *pos, *tlv, *pos_address, *pos_next_address;
	int length, len_eth, tlv_type, tlv_len, addresses_len, address_len;
#ifdef ENABLE_DOT1
	struct lldpd_vlan *vlan;
#endif
	log_debug("cdp", "decode CDP frame received on %s",
	    hardware->h_ifname);
	if ((chassis = calloc(1, sizeof(struct lldpd_chassis))) == NULL) {
		log_warn("cdp", "failed to allocate remote chassis");
		return -1;
	}
	TAILQ_INIT(&chassis->c_mgmt);
	if ((port = calloc(1, sizeof(struct lldpd_port))) == NULL) {
		log_warn("cdp", "failed to allocate remote port");
		free(chassis);
		return -1;
	}
#ifdef ENABLE_DOT1
	TAILQ_INIT(&port->p_vlans);
#endif
	length = s;
	pos = (u_int8_t*)frame;
	if (length < 2*ETHER_ADDR_LEN + sizeof(u_int16_t)   +
	    8   + 4  ) {
		log_warn("cdp", "too short CDP/FDP frame received on %s", hardware->h_ifname);
		goto malformed;
	}
	if (PEEK_CMP(cdpaddr, sizeof(cdpaddr)) != 0) {
#ifdef ENABLE_FDP
		PEEK_RESTORE((u_int8_t*)frame);
		if (PEEK_CMP(fdpaddr, sizeof(fdpaddr)) != 0)
			fdp = 1;
		else {
#endif
			log_info("cdp", "frame not targeted at CDP/FDP multicast address received on %s",
			    hardware->h_ifname);
			goto malformed;
#ifdef ENABLE_FDP
		}
#endif
	}
	PEEK_DISCARD(ETHER_ADDR_LEN);	 
	len_eth = PEEK_UINT16;
	if (len_eth > length) {
		log_warnx("cdp", "incorrect 802.3 frame size reported on %s",
		    hardware->h_ifname);
		goto malformed;
	}
	PEEK_DISCARD(6);	 
	proto = PEEK_UINT16;
	if (proto != LLC_PID_CDP) {
		if ((proto != LLC_PID_DRIP) &&
		    (proto != LLC_PID_PAGP) &&
		    (proto != LLC_PID_PVSTP) &&
		    (proto != LLC_PID_UDLD) &&
		    (proto != LLC_PID_VTP) &&
		    (proto != LLC_PID_DTP) &&
		    (proto != LLC_PID_STP))
			log_debug("cdp", "incorrect LLC protocol ID received on %s",
			    hardware->h_ifname);
		goto malformed;
	}
#if 0
	cksum = frame_checksum(pos, len_eth - 8,
#ifdef ENABLE_FDP
	    !fdp		 
#else
	    1			 
#endif
		);
	if (cksum != 0) {
		log_info("cdp", "incorrect CDP/FDP checksum for frame received on %s (%d)",
			  hardware->h_ifname, cksum);
		goto malformed;
	}
#endif
	version = PEEK_UINT8;
	if ((version != 1) && (version != 2)) {
		log_warnx("cdp", "incorrect CDP/FDP version (%d) for frame received on %s",
		    version, hardware->h_ifname);
		goto malformed;
	}
	chassis->c_ttl = PEEK_UINT8;  
	PEEK_DISCARD_UINT16;	      
	while (length) {
		if (length < 4) {
			log_warnx("cdp", "CDP/FDP TLV header is too large for "
			    "frame received on %s",
			    hardware->h_ifname);
			goto malformed;
		}
		tlv_type = PEEK_UINT16;
		tlv_len = PEEK_UINT16 - 4;
		(void)PEEK_SAVE(tlv);
		if ((tlv_len < 0) || (length < tlv_len)) {
			log_warnx("cdp", "incorrect size in CDP/FDP TLV header for frame "
			    "received on %s",
			    hardware->h_ifname);
			goto malformed;
		}
		switch (tlv_type) {
		case CDP_TLV_CHASSIS:
			if ((chassis->c_name = (char *)calloc(1, tlv_len + 1)) == NULL) {
				log_warn("cdp", "unable to allocate memory for chassis name");
				goto malformed;
			}
			PEEK_BYTES(chassis->c_name, tlv_len);
			chassis->c_id_subtype = LLDP_CHASSISID_SUBTYPE_LOCAL;
			if ((chassis->c_id =  (char *)malloc(tlv_len)) == NULL) {
				log_warn("cdp", "unable to allocate memory for chassis ID");
				goto malformed;
			}
			memcpy(chassis->c_id, chassis->c_name, tlv_len);
			chassis->c_id_len = tlv_len;
			break;
		case CDP_TLV_ADDRESSES:
			CHECK_TLV_SIZE(4, "Address");
			addresses_len = tlv_len - 4;
			for (nb = PEEK_UINT32; nb > 0; nb--) {
				(void)PEEK_SAVE(pos_address);
				if (addresses_len < 2) {
					log_warn("cdp", "too short address subframe "
						  "received on %s",
						  hardware->h_ifname);
					goto malformed;
				}
				PEEK_DISCARD_UINT8; addresses_len--;
				address_len = PEEK_UINT8; addresses_len--;
				if (addresses_len < address_len + 2) {
					log_warn("cdp", "too short address subframe "
						  "received on %s",
						  hardware->h_ifname);
					goto malformed;
				}
				PEEK_DISCARD(address_len);
				addresses_len -= address_len;
				address_len = PEEK_UINT16; addresses_len -= 2;
				if (addresses_len < address_len) {
					log_warn("cdp", "too short address subframe "
						  "received on %s",
						  hardware->h_ifname);
					goto malformed;
				}
				PEEK_DISCARD(address_len);
				(void)PEEK_SAVE(pos_next_address);
				PEEK_RESTORE(pos_address);
				if ((PEEK_UINT8 == 1) && (PEEK_UINT8 == 1) &&
				    (PEEK_UINT8 == CDP_ADDRESS_PROTO_IP) &&
				    (PEEK_UINT16 == sizeof(struct in_addr))) {
						PEEK_BYTES(&addr, sizeof(struct in_addr));
						mgmt = lldpd_alloc_mgmt(LLDPD_AF_IPV4, &addr, 
									sizeof(struct in_addr), 0);
						if (mgmt == NULL) {
							assert(errno == ENOMEM);
							log_warn("cdp", "unable to allocate memory for management address");
							goto malformed;
						}
						TAILQ_INSERT_TAIL(&chassis->c_mgmt, mgmt, m_entries);
				}
				PEEK_RESTORE(pos_next_address);
			}
			break;
		case CDP_TLV_PORT:
			if (tlv_len == 0) {
				log_warn("cdp", "too short port description received");
				goto malformed;
			}
			if ((port->p_descr = (char *)calloc(1, tlv_len + 1)) == NULL) {
				log_warn("cdp", "unable to allocate memory for port description");
				goto malformed;
			}
			PEEK_BYTES(port->p_descr, tlv_len);
			port->p_id_subtype = LLDP_PORTID_SUBTYPE_IFNAME;
			if ((port->p_id =  (char *)calloc(1, tlv_len)) == NULL) {
				log_warn("cdp", "unable to allocate memory for port ID");
				goto malformed;
			}
			memcpy(port->p_id, port->p_descr, tlv_len);
			port->p_id_len = tlv_len;
			break;
		case CDP_TLV_CAPABILITIES:
#ifdef ENABLE_FDP
			if (fdp) {
				if (!strncmp("Router", (char*)pos, tlv_len))
					chassis->c_cap_enabled = LLDP_CAP_ROUTER;
				else if (!strncmp("Switch", (char*)pos, tlv_len))
					chassis->c_cap_enabled = LLDP_CAP_BRIDGE;
				else if (!strncmp("Bridge", (char*)pos, tlv_len))
					chassis->c_cap_enabled = LLDP_CAP_REPEATER;
				else
					chassis->c_cap_enabled = LLDP_CAP_STATION;
				chassis->c_cap_available = chassis->c_cap_enabled;
				break;
			}
#endif
			CHECK_TLV_SIZE(4, "Capabilities");
			caps = PEEK_UINT32;
			if (caps & CDP_CAP_ROUTER)
				chassis->c_cap_enabled |= LLDP_CAP_ROUTER;
			if (caps & 0x0e)
				chassis->c_cap_enabled |= LLDP_CAP_BRIDGE;
			if (chassis->c_cap_enabled == 0)
				chassis->c_cap_enabled = LLDP_CAP_STATION;
			chassis->c_cap_available = chassis->c_cap_enabled;
			break;
		case CDP_TLV_SOFTWARE:
			software_len = tlv_len;
			(void)PEEK_SAVE(software);
			break;
		case CDP_TLV_PLATFORM:
			platform_len = tlv_len;
			(void)PEEK_SAVE(platform);
			break;
#ifdef ENABLE_DOT1
		case CDP_TLV_NATIVEVLAN:
			CHECK_TLV_SIZE(2, "Native VLAN");
			if ((vlan = (struct lldpd_vlan *)calloc(1,
				sizeof(struct lldpd_vlan))) == NULL) {
				log_warn("cdp", "unable to alloc vlan "
					  "structure for "
					  "tlv received on %s",
					  hardware->h_ifname);
				goto malformed;
			}
			vlan->v_vid = port->p_pvid = PEEK_UINT16;
			if (asprintf(&vlan->v_name, "VLAN #%d", vlan->v_vid) == -1) {
				log_warn("cdp", "unable to alloc VLAN name for "
					  "TLV received on %s",
					  hardware->h_ifname);
				free(vlan);
				goto malformed;
			}
			TAILQ_INSERT_TAIL(&port->p_vlans,
					  vlan, v_entries);
			break;
#endif
		default:
			log_debug("cdp", "unknown CDP/FDP TLV type (%d) received on %s",
			    ntohs(tlv_type), hardware->h_ifname);
			hardware->h_rx_unrecognized_cnt++;
		}
		PEEK_DISCARD(tlv + tlv_len - pos);
	}
	if (!software && platform) {
		if ((chassis->c_descr = (char *)calloc(1,
			    platform_len + 1)) == NULL) {
			log_warn("cdp", "unable to allocate memory for chassis description");
			goto malformed;
		}
		memcpy(chassis->c_descr, platform, platform_len);
	} else if (software && !platform) {
		if ((chassis->c_descr = (char *)calloc(1,
			    software_len + 1)) == NULL) {
			log_warn("cdp", "unable to allocate memory for chassis description");
			goto malformed;
		}
		memcpy(chassis->c_descr, software, software_len);
	} else if (software && platform) {
#define CONCAT_PLATFORM " running on\n"
		if ((chassis->c_descr = (char *)calloc(1,
			    software_len + platform_len +
			    strlen(CONCAT_PLATFORM) + 1)) == NULL) {
			log_warn("cdp", "unable to allocate memory for chassis description");
			goto malformed;
		}
		memcpy(chassis->c_descr, platform, platform_len);
		memcpy(chassis->c_descr + platform_len,
		    CONCAT_PLATFORM, strlen(CONCAT_PLATFORM));
		memcpy(chassis->c_descr + platform_len + strlen(CONCAT_PLATFORM),
		    software, software_len);
	}
	if ((chassis->c_id == NULL) ||
	    (port->p_id == NULL) ||
	    (chassis->c_name == NULL) ||
	    (chassis->c_descr == NULL) ||
	    (port->p_descr == NULL) ||
	    (chassis->c_ttl == 0) ||
	    (chassis->c_cap_enabled == 0)) {
		log_warnx("cdp", "some mandatory CDP/FDP tlv are missing for frame received on %s",
		    hardware->h_ifname);
		goto malformed;
	}
	*newchassis = chassis;
	*newport = port;
	return 1;
malformed:
	lldpd_chassis_cleanup(chassis, 1);
	lldpd_port_cleanup(port, 1);
	free(port);
	return -1;
}