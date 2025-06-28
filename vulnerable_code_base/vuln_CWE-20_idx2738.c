error_t dm9000ReceivePacket(NetInterface *interface)
{
   error_t error;
   size_t i;
   size_t n;
   size_t length;
   volatile uint8_t status;
   volatile uint16_t data;
   Dm9000Context *context;
   context = (Dm9000Context *) interface->nicContext;
   data = dm9000ReadReg(DM9000_REG_MRCMDX);
   DM9000_INDEX_REG = DM9000_REG_MRCMDX1;
   status = LSB(DM9000_DATA_REG);
   if(status == 0x01)
   {
      DM9000_INDEX_REG = DM9000_REG_MRCMD;
      status = MSB(DM9000_DATA_REG);
      length = DM9000_DATA_REG;
      n = MIN(length, ETH_MAX_FRAME_SIZE);
      i = 0;
      if((status & (RSR_LCS | RSR_RWTO | RSR_PLE | RSR_AE | RSR_CE | RSR_FOE)) == 0)
      {
         while((i + 1) < n)
         {
            data = DM9000_DATA_REG;
            context->rxBuffer[i++] = LSB(data);
            context->rxBuffer[i++] = MSB(data);
         }
         if((i + 1) == n)
         {
            data = DM9000_DATA_REG;
            context->rxBuffer[i] = LSB(data);
            i += 2;
         }
         error = NO_ERROR;
      }
      else
      {
         error = ERROR_INVALID_PACKET;
      }
      while(i < length)
      {
         data = DM9000_DATA_REG;
         i += 2;
      }
   }
   else
   {
      error = ERROR_BUFFER_EMPTY;
   }
   if(!error)
   {
      NetRxAncillary ancillary;
      ancillary = NET_DEFAULT_RX_ANCILLARY;
      nicProcessPacket(interface, context->rxBuffer, n, &ancillary);
   }
   return error;
}