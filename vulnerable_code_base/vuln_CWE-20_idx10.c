void enc624j600EventHandler(NetInterface *interface)
{
   error_t error;
   uint16_t status;
   uint16_t value;
   status = enc624j600ReadReg(interface, ENC624J600_REG_EIR);
   if((status & EIR_LINKIF) != 0)
   {
      enc624j600ClearBit(interface, ENC624J600_REG_EIR, EIR_LINKIF);
      value = enc624j600ReadReg(interface, ENC624J600_REG_ESTAT);
      if((value & ESTAT_PHYLNK) != 0)
      {
         value = enc624j600ReadPhyReg(interface, ENC624J600_PHY_REG_PHSTAT3);
         if((value & PHSTAT3_SPDDPX1) != 0)
         {
            interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
         }
         else
         {
            interface->linkSpeed = NIC_LINK_SPEED_10MBPS;
         }
         if((value & PHSTAT3_SPDDPX2) != 0)
         {
            interface->duplexMode = NIC_FULL_DUPLEX_MODE;
         }
         else
         {
            interface->duplexMode = NIC_HALF_DUPLEX_MODE;
         }
         interface->linkState = TRUE;
         enc624j600UpdateMacConfig(interface);
      }
      else
      {
         interface->linkState = FALSE;
      }
      nicNotifyLinkChange(interface);
   }
   if((status & EIR_PKTIF) != 0)
   {
      enc624j600ClearBit(interface, ENC624J600_REG_EIR, EIR_PKTIF);
      do
      {
         error = enc624j600ReceivePacket(interface);
      } while(error != ERROR_BUFFER_EMPTY);
   }
   enc624j600SetBit(interface, ENC624J600_REG_EIE, EIE_LINKIE | EIE_PKTIE);
}