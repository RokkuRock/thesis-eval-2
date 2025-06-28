void enc28j60EventHandler(NetInterface *interface)
{
   error_t error;
   uint16_t status;
   uint16_t value;
   status = enc28j60ReadReg(interface, ENC28J60_REG_EIR);
   if((status & EIR_LINKIF) != 0)
   {
      enc28j60ReadPhyReg(interface, ENC28J60_PHY_REG_PHIR);
      enc28j60ClearBit(interface, ENC28J60_REG_EIR, EIR_LINKIF);
      value = enc28j60ReadPhyReg(interface, ENC28J60_PHY_REG_PHSTAT2);
      if((value & PHSTAT2_LSTAT) != 0)
      {
         interface->linkSpeed = NIC_LINK_SPEED_10MBPS;
#if (ENC28J60_FULL_DUPLEX_SUPPORT == ENABLED)
         interface->duplexMode = NIC_FULL_DUPLEX_MODE;
#else
         interface->duplexMode = NIC_HALF_DUPLEX_MODE;
#endif
         interface->linkState = TRUE;
      }
      else
      {
         interface->linkState = FALSE;
      }
      nicNotifyLinkChange(interface);
   }
   if((status & EIR_PKTIF) != 0)
   {
      enc28j60ClearBit(interface, ENC28J60_REG_EIR, EIR_PKTIF);
      do
      {
         error = enc28j60ReceivePacket(interface);
      } while(error != ERROR_BUFFER_EMPTY);
   }
   enc28j60SetBit(interface, ENC28J60_REG_EIE, EIE_LINKIE | EIE_PKTIE);
}