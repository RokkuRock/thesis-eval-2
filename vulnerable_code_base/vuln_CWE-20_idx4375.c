void stm32h7xxEthInitGpio(NetInterface *interface)
{
   GPIO_InitTypeDef GPIO_InitStructure;
#if defined(USE_STM32H743I_EVAL) || defined(USE_STM32H747I_EVAL) || \
   defined(USE_STM32H747I_DISCO)
   __HAL_RCC_SYSCFG_CLK_ENABLE();
   __HAL_RCC_GPIOA_CLK_ENABLE();
   __HAL_RCC_GPIOC_CLK_ENABLE();
   __HAL_RCC_GPIOG_CLK_ENABLE();
   HAL_SYSCFG_ETHInterfaceSelect(SYSCFG_ETH_RMII);
   GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStructure.Alternate = GPIO_AF11_ETH;
   GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
   GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
   GPIO_InitStructure.Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
   HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
#elif defined(USE_STM32H745I_DISCO) || defined(USE_STM32H750B_DISCO)
   __HAL_RCC_SYSCFG_CLK_ENABLE();
   __HAL_RCC_GPIOA_CLK_ENABLE();
   __HAL_RCC_GPIOB_CLK_ENABLE();
   __HAL_RCC_GPIOC_CLK_ENABLE();
   __HAL_RCC_GPIOE_CLK_ENABLE();
   __HAL_RCC_GPIOG_CLK_ENABLE();
   __HAL_RCC_GPIOH_CLK_ENABLE();
   __HAL_RCC_GPIOI_CLK_ENABLE();
   HAL_SYSCFG_ETHInterfaceSelect(SYSCFG_ETH_MII);
   GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStructure.Alternate = GPIO_AF11_ETH;
   GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
   GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
   GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
   GPIO_InitStructure.Pin = GPIO_PIN_2;
   HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
   GPIO_InitStructure.Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
   HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
   GPIO_InitStructure.Pin = GPIO_PIN_10;
   HAL_GPIO_Init(GPIOI, &GPIO_InitStructure);
#elif defined(USE_STM32H7XX_NUCLEO_144) || defined(USE_STM32H7XX_NUCLEO_144_MB1363) || \
   defined(USE_STM32H7XX_NUCLEO_144_MB1364)
   __HAL_RCC_SYSCFG_CLK_ENABLE();
   __HAL_RCC_GPIOA_CLK_ENABLE();
   __HAL_RCC_GPIOB_CLK_ENABLE();
   __HAL_RCC_GPIOC_CLK_ENABLE();
   __HAL_RCC_GPIOG_CLK_ENABLE();
   HAL_SYSCFG_ETHInterfaceSelect(SYSCFG_ETH_RMII);
   GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStructure.Alternate = GPIO_AF11_ETH;
   GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
   GPIO_InitStructure.Pin = GPIO_PIN_13;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
   GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
   GPIO_InitStructure.Pin = GPIO_PIN_11 | GPIO_PIN_13;
   HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
#endif
}