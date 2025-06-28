void dm9000EventHandler(NetInterface *interface)
{
   error_t error;
   uint8_t status;
   status = dm9000ReadReg(DM9000_REG_ISR);
   if((status & ISR_LNKCHG) != 0)
   {
      dm9000WriteReg(DM9000_REG_ISR, ISR_LNKCHG);
      status = dm9000ReadReg(DM9000_REG_NSR);
      if((status & NSR_LINKST) != 0)
      {
         if((status & NSR_SPEED) != 0)
         {
            interface->linkSpeed = NIC_LINK_SPEED_10MBPS;
         }
         else
         {
            interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
         }
         status = dm9000ReadReg(DM9000_REG_NCR);
         if((status & NCR_FDX) != 0)
         {
            interface->duplexMode = NIC_FULL_DUPLEX_MODE;
         }
         else
         {
            interface->duplexMode = NIC_HALF_DUPLEX_MODE;
         }
         interface->linkState = TRUE;
      }
      else
      {
         interface->linkState = FALSE;
      }
      nicNotifyLinkChange(interface);
   }
   if((status & ISR_PR) != 0)
   {
      dm9000WriteReg(DM9000_REG_ISR, ISR_PR);
      do
      {
         error = dm9000ReceivePacket(interface);
      } while(error != ERROR_BUFFER_EMPTY);
   }
   dm9000WriteReg(DM9000_REG_IMR, IMR_PAR | IMR_LNKCHGI | IMR_PTI | IMR_PRI);
}