#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_spi.h>
#include "spi.h"

static const uint16_t speeds[] = {
  [SPI_SLOW] = SPI_BaudRatePrescaler_64,
  [SPI_MEDIUM] = SPI_BaudRatePrescaler_8,
  [SPI_FAST] = SPI_BaudRatePrescaler_2};

void spiInit(SPI_TypeDef *SPIx)
{
  GPIO_InitTypeDef GPIO_InitStructureSCK;
  GPIO_InitTypeDef GPIO_InitStructureMISO;
  GPIO_InitTypeDef GPIO_InitStructureMOSI;
  SPI_InitTypeDef SPI_InitStructure;

  GPIO_StructInit(&GPIO_InitStructureSCK);
  GPIO_StructInit(&GPIO_InitStructureMISO);
  GPIO_StructInit(&GPIO_InitStructureMOSI);
  SPI_StructInit(&SPI_InitStructure);

  if(SPIx == SPI1)
  {
    /* Enable clocks, configure pins */
    // Enable clocks
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    // Configure pins
    GPIO_InitStructureSCK.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructureSCK.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructureSCK.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructureSCK);

    // **** in this LCD example we use write only and PA6 is used elsewhere ****
    //GPIO_InitStructureMISO.GPIO_Pin = GPIO_Pin_6;
    //GPIO_InitStructureMISO.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    //GPIO_Init(GPIOA, &GPIO_InitStructureMISO);

    GPIO_InitStructureMOSI.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructureMOSI.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructureMOSI.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructureMOSI);
   }
  else
  {  //  other SPI devices --
   return;
  }

  // Configure device
  SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;  //Write only for LCD demo
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; 
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = speeds[SPI_SLOW];  //*Use SLOW for LCD
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPIx, &SPI_InitStructure);

  SPI_Cmd(SPI1, ENABLE);
}

int spiReadWrite(SPI_TypeDef* SPIx, uint8_t *rbuf, 
         const uint8_t *tbuf, int cnt, enum spiSpeed speed)
{
  int i;

  SPIx->CR1 = (SPIx->CR1 & ~SPI_BaudRatePrescaler_256)|speeds[speed];

  for(i = 0; i < cnt; i++)
    {
	 while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) != SET);

     if (tbuf)
       SPI_I2S_SendData(SPIx, *tbuf++);
     else
       SPI_I2S_SendData(SPIx, 0xff);

     while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);

     if(rbuf)
       *rbuf++ = SPI_I2S_ReceiveData(SPIx);
     else
       SPI_I2S_ReceiveData(SPIx);

     /* Wait until the transmit buffer is empty */
     while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) != SET);

     /* wait until the completion of the transfer */
     while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_BSY) != RESET);
  }

  return i;
}

// ***********************************************************************
// spiWrite8(data)  write 8 bits (trimmed down for speed)
// ***********************************************************************
void spiWrite8( uint8_t data)
{
  while((SPI1->SR & SPI_I2S_FLAG_TXE) ==0);
  SPI1->DR = data;
  while((SPI1->SR & SPI_I2S_FLAG_BSY) != 0);
}

int spiReadWrite16(SPI_TypeDef* SPIx, uint16_t *rbuf, 
     const uint16_t *tbuf, int cnt, enum spiSpeed speed)
{
  int i;
  SPI_DataSizeConfig(SPIx, SPI_DataSize_16b);
  SPIx->CR1 = (SPIx->CR1 & ~SPI_BaudRatePrescaler_256) | speeds[speed];

  for(i = 0; i < cnt; i++)
    {
	 while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) != SET);

     if (tbuf)
       SPI_I2S_SendData(SPIx, *tbuf++);
     else
       SPI_I2S_SendData(SPIx, 0xffff);

     while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);

     if(rbuf)
       *rbuf++ = SPI_I2S_ReceiveData(SPIx);
     else
       SPI_I2S_ReceiveData(SPIx);

     /* Wait until the transmit buffer is empty */
     while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) != SET);

     /* wait until the completion of the transfer */
     while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_BSY) != RESET);
  }

  return i;
}



