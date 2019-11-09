// ***********************************************************************
// STM32F013_24C02_i2c - I2C communication with a AT24C04 EEPROM
// Based on - Template for USART Menu with GPIO & SysTick support
// Using USART1 PA10=RX PA9=TX, SysTick interrupts every 10us
//
// SDA=PB7 (pin 5/yellow), SCL=PB6 (pin 6/green) (pins 2,3&7=LOW, 1=High)
// pull-ups ARE REQUIRED on SDA & SCL lines  (10K - +5v or +3V)
// ATMEL AT24C02 Pins 1=A0, 2=A1, 3=A2, 4=GND, 5=SDA, 6=SCL 7=WP 8=VCC(+5V/3.3v)
//
// By Eric M. Klaus   2/2019
// ***********************************************************************
#include "stdio.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_i2c.h"

#define LED_PIN  GPIO_Pin_13
#define LED_2PIN GPIO_Pin_15
#define LED_PORT GPIOC

#define SCL_PIN  GPIO_Pin_8
#define SCL_PORT GPIOB
#define SDA_PIN  GPIO_Pin_9
#define SDA_PORT GPIOB

#define I2C_PERIPH RCC_APB2Periph_GPIOB

#define SLAVE_ADDRESS 0xA2
#define I2C_TIMEOUT_MAX 100000

/***************************************************************************//**
 * Declare function prototypes
 ******************************************************************************/
void showMenu(void);
void processMenuCmd(char cmd);
void dump16(uint8_t *buf);

// ***********************************************************************
// *** 24C02/TWI Function Definitions
// ***********************************************************************
void I2C_init(void);
uint8_t I2C_write_data(uint8_t addr, uint8_t *indata, uint8_t bytes);
uint8_t I2C_read_bytes(uint8_t addr, uint8_t *data, uint8_t bytes);


// SysTick based delay routines
void SysTick_Handler(void);

// Looping style delay routines (still controlled by SysTick)
void Delay_us(const uint32_t usec);
void Delay_ms(const uint32_t msec);

uint8_t AsciiToHexVal(char b);
uint8_t HexStrToByte(char *buf);

// USART routines ** specific to USART1
void SetupUSART(void);
void USART_PutChar(char c);
void USART_PutStr(char *str);
void USART_PutHexByte(unsigned char byte);
int USART_GetStr(char *buf, int len);

// Global timing counters
static __IO uint32_t TimingDelay, ctMs;
static __IO uint32_t usCounter, msCounter;

GPIO_InitTypeDef GPIO_InitStruct;

int led1_val=0;  // state of LED
char msg[80]; // general purpose string buffer

int main(void)
{
 int k=0;
 // At this point startup has executed invoking SystemInit()
 // which selects the HSE clock and configures it to run at 74MHz.

 // ** Configures the SysTick event to fire every 10us	**
 SysTick_Config(SystemCoreClock / 100000);

 // Configure the GPIO pin for the LED (PC13)
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);// Clock PORTB Enable
 GPIO_InitStruct.GPIO_Pin = LED_PIN | LED_2PIN; // Configure Led pins
 GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
 GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;// Set Output Push-Pull
 GPIO_Init(LED_PORT, &GPIO_InitStruct);

 SetupUSART();

 GPIO_ResetBits(LED_PORT, LED_PIN);  //LED ON
 GPIO_SetBits(LED_PORT, LED_2PIN);  //LED2 ON
 TimingDelay = 50000;                // Blink every 1/2 sec.

 showMenu();  // Display the user menu

 while(1)
   {
    if(TimingDelay==0)
	  {
	   TimingDelay = 50000;  //Reset the 1/2 sec. delay
	   if(led1_val)
	     {
          GPIO_ResetBits(LED_PORT, LED_PIN); // LED1 OFF
		  led1_val=0;
	     }
	   else
	     {
		  GPIO_SetBits(LED_PORT, LED_PIN); // LED1 ON
		  led1_val=1;
	     }

	   // ** add your code here to run every 1/2 second... ***
	  }

	// Is a byte available from UART?
	if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)
	  {
		k = USART_ReceiveData(USART1);
		USART_PutChar(k);
		processMenuCmd(k);
	  }

	// ** add your code to do something useful here .. ***
   }
}

// ***********************************************************************
// showMenu()    Output the user menu to the USART
// ***********************************************************************
void showMenu(void)
{
  USART_PutStr("\r\n **** USARTx Menu **** \r\n");
  USART_PutStr(" 1. Read 16 Bytes \r\n");
  USART_PutStr(" 2. Write 8 Bytes \r\n");
  USART_PutStr(" 3. Option Three \r\n");
  USART_PutStr(" 4. Option Four \r\n");
  USART_PutStr(" M. Re-Display Menu \r\n");
}

// ***********************************************************************
// processMenuCmd(cmd)   process the users selection
// ***********************************************************************
void processMenuCmd(char cmd)
{
  int x;
  uint8_t c, addr, in_buf[17];

  switch(cmd)
  {
   case '1':

	   USART_PutStr("\r\n Reading 16 Bytes...\r\n");

       I2C_init();

       USART_PutStr("\r\n  Enter Address: ");
       USART_GetStr(msg, 2);
       addr = HexStrToByte(msg);

       c = I2C_read_bytes(addr, in_buf, 16);
       if(c > 0 )
         {
    	  USART_PutStr("\r\n I2C_read_bytes() returned ERROR CODE: ");
    	  USART_PutHexByte(c);
    	  USART_PutStr("\r\n");
          break;
         }


       USART_PutStr("\r\n ");
       dump16(in_buf);

       //twi_disable();
       USART_PutStr("\r\n Done...\r\n");
   break;
   case '2':
	   I2C_init();
	   USART_PutStr("\r\n  Enter Write  Address:");
       USART_GetStr(msg, 2);
       addr = HexStrToByte(msg);
       USART_PutHexByte(addr);

	   USART_PutStr("\r\n  Enter Data:");
       x = USART_GetStr(msg, 8);

       c = I2C_write_data(addr, (uint8_t *)msg, (uint8_t)x);
       if(c > 0 )
         {
    	  USART_PutStr("\r\n I2C_write_data() returned ERROR CODE: ");
    	  USART_PutHexByte(c);
    	  USART_PutStr("\r\n");
          break;
         }

       //twi_disable();
       USART_PutStr("\r\n OK Done...\r\n");
   break;
   case '3':
	   USART_PutStr("\r\n  You Selected Option Three !!! \r\n");
   break;
   case '4':
	   USART_PutStr("\r\n  Blink LED...  \r\n");
	   for(x=1; x<20; x++)
	     {
		   GPIO_SetBits(LED_PORT, LED_2PIN); // LED1 ON
		   Delay_ms(100);
		   GPIO_ResetBits(LED_PORT, LED_2PIN); // LED1 OFF
		   Delay_ms(100);
	     }
	   USART_PutStr(" Done...\r\n");
   break;
   case 'M':
   case 'm':
	   showMenu();
   break;
  }

}

// ***********************************************************************
//  dump16(buf) display 16 bytes in hex & ascii
// ***********************************************************************
void dump16(uint8_t *buf)
{
 uint8_t x, c;
 for(x=0; x<16; x++)
   {
	 USART_PutHexByte(buf[x]);  // print HEX value
	 if(x <15)
		 USART_PutChar(',');    // followed by a comma...
	 else
		 USART_PutStr("   ");   // last byte gets no comma but 4 spaces
   }

 for(x=0; x<16; x++)
   {
	c = buf[x];
	if((c > 0x19)&&(c < 0x80))  // if printable, print char value
		 USART_PutChar(c);
	 else
		 USART_PutChar('.');   // non printable just print a dot
   }

}

// ***********************************************************************
// ************* TWI FUNCTIONS ******************************************
// ***********************************************************************

// ***********************************************************
// initialize I2C master mode
// set pin I/O direction, Alternate Function & configure I2C mode
// ***********************************************************
void I2C_init(void)
{
 I2C_InitTypeDef I2C_InitStruct; // this is for the I2C1 initialization

 RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE); // enable APB1 peripheral clock for I2C1
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);// Clock PORTB Enable

 GPIO_StructInit(&GPIO_InitStruct);
 GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // Pins 6(I2C1_SCL) and 7(I2C1_SDA)
 GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;   // the pins are configured as alternate function so the I2C peripheral has access to them
 GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; // this defines the GPIO speed and has nothing to do with the data rate!
 GPIO_Init(SDA_PORT, &GPIO_InitStruct);         // now all the values are passed to the GPIO_Init()

  /* Set the I2C structure parameters */
 I2C_StructInit( &I2C_InitStruct);
 I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
 I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
 I2C_InitStruct.I2C_OwnAddress1 = 0x38;          // not really important for this demo
 I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
 I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
 I2C_InitStruct.I2C_ClockSpeed = 100000;

 I2C_Cmd(I2C1, ENABLE);

 /* Initialize the I2C peripheral w/ selected parameters */
 I2C_Init(I2C1, &I2C_InitStruct);

}

// ***********************************************************
// Writes data from buffer.
//    addr   Page address to write to
//    indata Pointer to data buffer
//    bytes  Number of bytes to transfer (NOTE: 8 max for 24C02)
//    Returns  0 if successful, otherwise error 1-4
// ***********************************************************
uint8_t I2C_write_data(uint8_t addr, uint8_t *indata, uint8_t bytes)
{
  uint8_t index;
  uint32_t timeout = I2C_TIMEOUT_MAX;

  I2C_AcknowledgeConfig(I2C1, ENABLE);  // Send ACK after each byte sent.

  I2C_GenerateSTART(I2C1, ENABLE);

  timeout = I2C_TIMEOUT_MAX;
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    	{
  	 if((timeout--) == 0)
  	   return 1;
  	}

  	I2C_Send7bitAddress(I2C1, SLAVE_ADDRESS, I2C_Direction_Transmitter);

  	timeout = I2C_TIMEOUT_MAX;
    	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    	{
  	 if((timeout--) == 0)
  		return 2;
    	}

  I2C_SendData(I2C1, addr);
  timeout = I2C_TIMEOUT_MAX;

  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
  	{
  	 if((timeout--) == 0)
  	   return 3;
    }

  for(index=0; index < bytes; index++)
  	{
   	  I2C_SendData(I2C1, indata[index]);

   	 timeout = I2C_TIMEOUT_MAX;
     while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
  	     {
  		  if ((timeout--) == 0)
  			  return 4;
    	 }
  	}

  	I2C_GenerateSTOP(I2C1, ENABLE);

  	return 0;

}


// ***********************************************************
// Reads data into buffer.
//    addr Address to read from
//    data Pointer to data buffer
//    bytes  Number of bytes to read
//    Returns  0 if successful, otherwise error code 1-6
// ***********************************************************
uint8_t I2C_read_bytes(uint8_t addr, uint8_t *data, uint8_t bytes)
{
 uint8_t index;
 uint32_t timeout = I2C_TIMEOUT_MAX;

 I2C_AcknowledgeConfig(I2C1, ENABLE);
 I2C_GenerateSTART(I2C1, ENABLE);

 timeout = I2C_TIMEOUT_MAX;
 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
   	{
 	 if((timeout--) == 0)
 	   return 1;
 	}

 	I2C_Send7bitAddress(I2C1, SLAVE_ADDRESS, I2C_Direction_Transmitter);

 	timeout = I2C_TIMEOUT_MAX;
   	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
   	{
 	 if((timeout--) == 0)
 		return 2;
   	}

 I2C_SendData(I2C1, addr);
 timeout = I2C_TIMEOUT_MAX;

 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
 	{
 	 if((timeout--) == 0)
 	   return 3;
   	}


 I2C_GenerateSTART(I2C1, ENABLE);
 timeout = I2C_TIMEOUT_MAX;
 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
   	{
 	 if((timeout--) == 0)
 		 return 4;
 	}

 I2C_Send7bitAddress(I2C1, SLAVE_ADDRESS, I2C_Direction_Receiver);

 timeout = I2C_TIMEOUT_MAX;
 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
   	{
 	 if ((timeout--) == 0)
 		 return 5;
   	}

 for(index = 0; index < bytes; index++)
	{
	 timeout = I2C_TIMEOUT_MAX;
	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))
	   	{
	 	 if((timeout--) == 0)
	 		return 7;
	   	}

	 data[index] = I2C_ReceiveData(I2C1);
	}

 I2C_GenerateSTOP(I2C1,ENABLE);
 I2C_AcknowledgeConfig(I2C1,DISABLE);

 return 0;
}

// ***********************************************************************
// ********************* HELPER FUNCTIONS ********************************
// ***********************************************************************

//************************************************************************
// AsciiToHexVal(char b)
// Convert a single ASCII character to it's 4 bit value 0-9 or A-F
// Note: no value error checking is done. Valid HEX characters is assumed
//************************************************************************
uint8_t AsciiToHexVal(char b)
{
 char v= b & 0x0F;  // '0'-'9' simply mask high 4 bits
 if(b>'9')
   v+=9;           // 'A'-'F' = low 4 bits +9
 return v;
}
//************************************************************************
// HexStrToByte(char *buf)
// Convert a 2 character ASCII string to a 8 bit unsigned value
//************************************************************************
uint8_t HexStrToByte(char *buf)
{
 uint8_t v;
 v= AsciiToHexVal(buf[0]) * 16;
 v+= AsciiToHexVal(buf[1]);
 return v;
}


// ***********************************************************************
// ********************* USART FUNCTIONS ********************************
// ***********************************************************************

// ***********************************************************************
//  SetupUSART()
//
// USART1 configured as follow:
// BaudRate = 9600 baud, Word Length = 8 Bits, One Stop Bit, No parity
// Hardware flow control disabled (RTS and CTS signals)
// Receive and transmit enabled
// USART Clock disabled
// USART CPOL: Clock is active low
// USART CPHA: Data is captured on the middle
// USART LastBit: The clock pulse of the last data bit is not output to
//                           the SCLK pin
// ***********************************************************************
void SetupUSART()
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 USART_InitTypeDef USART_InitStructure;

 // Enable USART1 and GPIOA clock
 RCC_APB2PeriphClockCmd (RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

 // Configure USART1 Rx (PA10) as input floating
 GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
 GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
 GPIO_Init(GPIOA, &GPIO_InitStructure);

 // Configure USART1 Tx (PA9) as alternate function push-pull            */
 GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
 GPIO_Init(GPIOA, &GPIO_InitStructure);

 USART_InitStructure.USART_BaudRate            = 9600;
 USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
 USART_InitStructure.USART_StopBits            = USART_StopBits_1;
 USART_InitStructure.USART_Parity              = USART_Parity_No ;
 USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
 USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
 USART_Init(USART1, &USART_InitStructure);
 USART_Cmd(USART1, ENABLE);
}

// ***********************************************************************
// ******** USART Utility functions **************************************
// ***********************************************************************
// ***********************************************************************
// USART_PutChar(char *c)
// ***********************************************************************
void USART_PutChar(char c)
{
  // write a character to the USART
  USART_SendData(USART1, (uint8_t) c);

  ///Loop until the end of transmission
  while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}

// ***********************************************************************
// USART_PutStr(char *str)
// ***********************************************************************
void USART_PutStr(char *str)
{
  while(*str)
  {
    USART_PutChar(*str);
	str++;
  }
}

// ***********************************************************************
// USART_PutHexByte(char byte)
// ***********************************************************************
void USART_PutHexByte(unsigned char byte)
{
  char n = (byte >> 4) & 0x0F;
  // Write high order digit
  if(n < 10)
 	USART_PutChar(n + '0');
  else
	USART_PutChar(n - 10 + 'A');

  // Write low order digit
  n = (byte & 0x0F);
  if(n < 10)
 	USART_PutChar(n + '0');
  else
	USART_PutChar(n - 10 + 'A');
}

// ***********************************************************************
//	USART_GetStr(char *buf, int len)
//  Return length of input string
// ***********************************************************************
int USART_GetStr(char *buf, int len)
{
 int i=0;
 char k = 0;

 while(k != 0x0D)
    {
	  // Is a byte available from UART?
	 if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)
	   {
		k = USART_ReceiveData(USART1);  // get input char
		USART_PutChar(k);               // echo input char
		buf[i]=k;                       // store input char

		if(++i==len)  // Buffer Full = EXIT
	      break;

		if((k==0x0A)||(k==0x1B)) // LF or Esc = EXIT
			break;

		if((k==0x7F)&&(i>0))  // Backspace
			i--;
	   }
    }

  buf[i]=0;
  return i;
}


// ***********************************************************************
// ******* SysTick based Delay functions  ********************************
// ***********************************************************************
// ***********************************************************************
//  Simple microsecond delay routine
// ***********************************************************************
void Delay_us(const uint32_t usec)
{
  TimingDelay = usec/10;
  while(TimingDelay != 0);
}

// ***********************************************************************
//  Simple millisecond delay routine
// ***********************************************************************
void Delay_ms(const uint32_t msec)
{
  Delay_us(msec * 1000);
}


// ***********************************************************************
//  SystemTick IRQ handler
//  Decrements TimingDelay, increments usCounter & msCounter;
// ***********************************************************************
void SysTick_Handler(void)
{
 if (TimingDelay != 0x00)
   {
     TimingDelay--;
   }

 usCounter+=10;
 if(++ctMs > 99)
   {
	msCounter++;
	ctMs=0;
   }
}



