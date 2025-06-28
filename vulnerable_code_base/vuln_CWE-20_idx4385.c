error_t enc624j600ReceivePacket(NetInterface *interface)
{
   error_t error;
   uint16_t n;
   uint32_t status;
   Enc624j600Context *context;
   context = (Enc624j600Context *) interface->nicContext;
   if(enc624j600ReadReg(interface, ENC624J600_REG_ESTAT) & ESTAT_PKTCNT)
   {
      enc624j600WriteReg(interface, ENC624J600_REG_ERXRDPT, context->nextPacket);
      enc624j600ReadBuffer(interface, ENC624J600_CMD_RRXDATA,
         (uint8_t *) &context->nextPacket, sizeof(uint16_t));
      context->nextPacket = letoh16(context->nextPacket);
      enc624j600ReadBuffer(interface, ENC624J600_CMD_RRXDATA,
         (uint8_t *) &n, sizeof(uint16_t));
      n = letoh16(n);
      enc624j600ReadBuffer(interface, ENC624J600_CMD_RRXDATA,
         (uint8_t *) &status, sizeof(uint32_t));
      status = letoh32(status);
      if((status & RSV_RECEIVED_OK) != 0)
      {
         n = MIN(n, ETH_MAX_FRAME_SIZE);
         enc624j600ReadBuffer(interface, ENC624J600_CMD_RRXDATA, context->rxBuffer, n);
         error = NO_ERROR;
      }
      else
      {
         error = ERROR_INVALID_PACKET;
      }
      if(context->nextPacket == ENC624J600_RX_BUFFER_START)
      {
         enc624j600WriteReg(interface, ENC624J600_REG_ERXTAIL, ENC624J600_RX_BUFFER_STOP);
      }
      else
      {
         enc624j600WriteReg(interface, ENC624J600_REG_ERXTAIL, context->nextPacket - 2);
      }
      enc624j600SetBit(interface, ENC624J600_REG_ECON1, ECON1_PKTDEC);
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