error_t enc28j60ReceivePacket(NetInterface *interface)
{
   error_t error;
   uint16_t n;
   uint16_t status;
   Enc28j60Context *context;
   context = (Enc28j60Context *) interface->nicContext;
   if(enc28j60ReadReg(interface, ENC28J60_REG_EPKTCNT))
   {
      enc28j60WriteReg(interface, ENC28J60_REG_ERDPTL, LSB(context->nextPacket));
      enc28j60WriteReg(interface, ENC28J60_REG_ERDPTH, MSB(context->nextPacket));
      enc28j60ReadBuffer(interface, (uint8_t *) &context->nextPacket, sizeof(uint16_t));
      enc28j60ReadBuffer(interface, (uint8_t *) &n, sizeof(uint16_t));
      enc28j60ReadBuffer(interface, (uint8_t *) &status, sizeof(uint16_t));
      if((status & RSV_RECEIVED_OK) != 0)
      {
         n = MIN(n, ETH_MAX_FRAME_SIZE);
         enc28j60ReadBuffer(interface, context->rxBuffer, n);
         error = NO_ERROR;
      }
      else
      {
         error = ERROR_INVALID_PACKET;
      }
      if(context->nextPacket == ENC28J60_RX_BUFFER_START)
      {
         enc28j60WriteReg(interface, ENC28J60_REG_ERXRDPTL, LSB(ENC28J60_RX_BUFFER_STOP));
         enc28j60WriteReg(interface, ENC28J60_REG_ERXRDPTH, MSB(ENC28J60_RX_BUFFER_STOP));
      }
      else
      {
         enc28j60WriteReg(interface, ENC28J60_REG_ERXRDPTL, LSB(context->nextPacket - 1));
         enc28j60WriteReg(interface, ENC28J60_REG_ERXRDPTH, MSB(context->nextPacket - 1));
      }
      enc28j60SetBit(interface, ENC28J60_REG_ECON2, ECON2_PKTDEC);
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