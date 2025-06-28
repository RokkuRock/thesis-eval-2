void enc28j60SelectBank(NetInterface *interface, uint16_t address)
{
   uint16_t bank;
   Enc28j60Context *context;
   context = (Enc28j60Context *) interface->nicContext;
   bank = address & REG_BANK_MASK;
   if(bank != context->currentBank)
   {
      switch(bank)
      {
      case BANK_0:
         enc28j60ClearBit(interface, ENC28J60_REG_ECON1, ECON1_BSEL1 | ECON1_BSEL0);
         break;
      case BANK_1:
         enc28j60SetBit(interface, ENC28J60_REG_ECON1, ECON1_BSEL0);
         enc28j60ClearBit(interface, ENC28J60_REG_ECON1, ECON1_BSEL1);
         break;
      case BANK_2:
         enc28j60ClearBit(interface, ENC28J60_REG_ECON1, ECON1_BSEL0);
         enc28j60SetBit(interface, ENC28J60_REG_ECON1, ECON1_BSEL1);
         break;
      case BANK_3:
         enc28j60SetBit(interface, ENC28J60_REG_ECON1, ECON1_BSEL1 | ECON1_BSEL0);
         break;
      default:
         break;
      }
      context->currentBank = bank;
   }
}