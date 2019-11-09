// ***********************************************************************
// STM32F103_UART_INTx
// Based on template for USART Menu with GPIO & SysTick support
// Using USART1 PA10=RX PA9=TX  and SysTick interrupts every 10us
// Adding RX and TX interrupt support
//
// ** Used this project as a template for other work ***
// By Eric M. Klaus   2/2016
// ***********************************************************************
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"

#define LED_PIN  GPIO_Pin_13
#define LED_PORT GPIOC


#define RXBUFSIZE  64       // MUST be power of 2 e.g. 2,4,8,16,32,64,128 or 256
#define TXBUFSIZE  64       // MUST be power of 2 e.g. 2,4,8,16,32,64,128 or 256


/***************************************************************************//**
 * Declare function prototypes
 ******************************************************************************/
void showMenu(void);
void processMenuCmd(char cmd);


// SysTick based delay routines
void SysTick_Handler(void);

// Looping style delay routines (still controlled by SysTick)
//    NOTE: provided as examples, none used in this example
void Delay_us(const uint32_t usec);
void Delay_ms(const uint32_t msec);

// USART routines ** specific to USART1
void SetupUSART(void);
uint8_t USART_Get_Char(void);
void USART_PutChar(char c);
void USART_PutStr(char *str);
void USART_PutHexByte(unsigned char byte);
int USART_GetStr(char *buf, int len);

// Global timing counters
static __IO uint32_t TimingDelay, ctMs;
static __IO uint32_t usCounter, msCounter;

//** global variablers for UART interrupt support **
static __IO uint8_t rxInPtr=0, rxOutPtr=0, txInPtr=0, txOutPtr=0;
static __IO uint8_t rxBuf[RXBUFSIZE], txBuf[TXBUFSIZE];


GPIO_InitTypeDef GPIO_InitStruct;

int led1_val=0;  // state of LED
char msgBuf[80]; // general purpose string buffer

int main(void)
{
 int k=0;
 // At this point startup has executed invoking SystemInit()
 // which selects the HSE clock and configures it to run at 74MHz.

 // ** Configures the SysTick event to fire every 10us	**
 SysTick_Config(SystemCoreClock / 100000);

 // Configure the GPIO pin for the LED (PC13)
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);// Clock PORTB Enable
 GPIO_InitStruct.GPIO_Pin = LED_PIN; // Configure Led pin
 GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
 GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;// Set Output Push-Pull
 GPIO_Init(LED_PORT, &GPIO_InitStruct);

 SetupUSART();

 GPIO_ResetBits(LED_PORT, LED_PIN);  //LED ON
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
	if(rxInPtr != rxOutPtr)
	  {
		k = USART_Get_Char();
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
  USART_PutStr(" 1. Option One \r\n");
  USART_PutStr(" 2. Option Two \r\n");
  USART_PutStr(" 3. Option Three \r\n");
  USART_PutStr(" 4. Option Four \r\n");
  USART_PutStr(" M. Re-Display Menu \r\n");
}

void processMenuCmd(char cmd)
{
  int x;
  switch(cmd)
  {
   case '1':
	   USART_PutStr("\r\n rxInPtr = 0x");
	   USART_PutHexByte(rxInPtr);
	   USART_PutStr("\r\n");
   break;
   case '2':
	   USART_PutStr("\r\n  You pressed TWO !! \r\n");
   break;
   case '3':
	   USART_PutStr("\r\n  You Selected Option Three !!! \r\n");
   break;
   case '4':
	   USART_PutStr("\r\n  Blink LED...  \r\n");
	   for(x=1; x<20; x++)
	     {
		   GPIO_SetBits(LED_PORT, LED_PIN); // LED1 ON
		   Delay_ms(100);
		   GPIO_ResetBits(LED_PORT, LED_PIN); // LED1 OFF
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
 NVIC_InitTypeDef NVIC_InitStructure;

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

 USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  // Enable RX interrupt only

 // ******* Enable the USART global Interrupt  **********
 NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

 NVIC_Init(&NVIC_InitStructure);


 USART_Cmd(USART1, ENABLE);
}


// ***********************************************************************
//  USART1_IRQHandler
//  Handles USART1 global interrupt request.
// ***********************************************************************
void USART1_IRQHandler(void)
{

 if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE))
   {                  // read interrupt
    rxBuf[rxInPtr]= USART_ReceiveData(USART1);   // Save the incoming byte
    rxInPtr++;                                   // Increment RX IN pointer
    rxInPtr &= (RXBUFSIZE-1);                    // Adjust for buffer rollover

    USART_ClearFlag(USART1, USART_FLAG_RXNE);    // clear interrupt flag
    return;                                      // skip TX logic
   }

    if(txInPtr != txOutPtr)  //Is TX data available for sending
      {
       if(USART_GetFlagStatus(USART1, USART_FLAG_TXE))
         {
    	  USART_SendData(USART1, (uint8_t) txBuf[txOutPtr]); // Send the next byte from txBuf[]
    	  txOutPtr++;                                        // Increment TX OUT pointer
    	  txOutPtr &= (TXBUFSIZE-1);                         // Adjust for buffer rollover
    	  USART_ClearFlag(USART1, USART_FLAG_TXE);           // clear interrupt
         }
      }
    else  // disable TX interrupt if nothing to send (* USART_PutChar() will turn it back on again)
      {
		USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
      }

}


// ***********************************************************************
// ******** USART Utility functions **************************************
// ***********************************************************************

// ***********************************************************************
// USART_Get_Char()
// Wait for data to be available and return 1 byte
// ***********************************************************************
uint8_t USART_Get_Char(void)
{
 uint8_t c;

 // wait for data to be received
 while(rxInPtr == rxOutPtr){};

 c = rxBuf[rxOutPtr];      // Grab the byte from the RX buffer
 rxOutPtr++;               // Increment the OUT pointer
 rxOutPtr &= RXBUFSIZE-1;  // Adjust for buffer rollover

 return c;
}

// ***********************************************************************
// USART_PutChar(char *c)
// ***********************************************************************
void USART_PutChar(char c)
{
 // test for buffer overflow (consider returning an error code?)
 while(((txInPtr+1) & (TXBUFSIZE-1)) == txOutPtr){};    // wait for overflow condition to clear

 txBuf[txInPtr] = c;           // save the byte in txBuf[]
 txInPtr++;                    // increment the tx IN pointer
 txInPtr &= (TXBUFSIZE-1);     // adjust for buffer rollover
 USART_ITConfig(USART1, USART_IT_TXE, ENABLE); // enable the TX interrupt
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
	 if(rxInPtr != rxOutPtr)
	   {
		k =USART_Get_Char();            // get input char
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



