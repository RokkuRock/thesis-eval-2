uint16_t lpc546xxEthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;
   uint32_t temp;
   if(opcode == SMI_OPCODE_READ)
   {
      temp = ENET->MAC_MDIO_ADDR & ENET_MAC_MDIO_ADDR_CR_MASK;
      temp |= ENET_MAC_MDIO_ADDR_MOC(3) | ENET_MAC_MDIO_ADDR_MB_MASK;
      temp |= ENET_MAC_MDIO_ADDR_PA(phyAddr);
      temp |= ENET_MAC_MDIO_ADDR_RDA(regAddr);
      ENET->MAC_MDIO_ADDR = temp;
      while((ENET->MAC_MDIO_ADDR & ENET_MAC_MDIO_ADDR_MB_MASK) != 0)
      {
      }
      data = ENET->MAC_MDIO_DATA & ENET_MAC_MDIO_DATA_MD_MASK;
   }
   else
   {
      data = 0;
   }
   return data;
}