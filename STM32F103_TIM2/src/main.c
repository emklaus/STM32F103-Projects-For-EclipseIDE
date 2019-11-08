// ***********************************************************************
// STM32F103_TIM2  - Based on Template for USART Menu with GPIO & SysTick support
// Using USART1 PA10=RX PA9=TX with SysTick interrupts every 10us
// Demo using TIMER2 to generate interrupt every 1ms.
//
// ** Used this project as a template for other work ***
// By Eric M. Klaus   9/2019
// ***********************************************************************
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"
#include "stdlib.h"  //for itoa(), atoi()

#define LED_PIN  GPIO_Pin_13
#define LED_PORT GPIOC

/***************************************************************************//**
 * Declare function prototypes
 ******************************************************************************/
void showMenu(void);
void processMenuCmd(char cmd);


// SysTick based delay routines
void SysTick_Handler(void);

// Looping style delay routines (still controlled by SysTick)
void Delay_us(const uint32_t usec);
void Delay_ms(const uint32_t msec);

// Timer 2 Init routine
void setupTIM2(void);


// USART routines ** specific to USART1
void SetupUSART(void);
void USART_PutChar(char c);
void USART_PutStr(char *str);
void USART_PutHexByte(unsigned char byte);
int USART_GetStr(char *buf, int len);

// Global timing counters
static __IO uint32_t TimingDelay, ctMs;
static __IO uint32_t usCounter, msCounter;
static __IO uint32_t blinkCounter, tm2MsCounter;

GPIO_InitTypeDef GPIO_InitStruct;

int bUseTimer2Blink=0;  // state of Timer2 IRQ Enabled
char msgBuf[80]; // general purpose string buffer

int main(void)
{
 int k=0;
 SystemInit();
 // At this point startup has executed invoking SystemInit()
 // which selects the HSE clock and configures it to run at 72MHz.

 // ** Configures the SysTick event to fire every 10us	**
 SysTick_Config(SystemCoreClock / 100000);

 // Configure the GPIO pin for the LED (PC13)
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); // Clock PORTC Enable
 GPIO_InitStruct.GPIO_Pin = LED_PIN;                   // Configure Led pin
 GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
 GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;         // Set pin as Output Push-Pull
 GPIO_Init(LED_PORT, &GPIO_InitStruct);

 SetupUSART();   // Configure USART1

 GPIO_ResetBits(LED_PORT, LED_PIN);   // LED ON
 TimingDelay = 50000;                 // Blink every 1/2 sec.

 showMenu();  // Display the user menu

 while(1)
   {
    if(TimingDelay==0)
	  {
	   TimingDelay = 50000;  //Reset the 1/2 sec. delay driven by SysTick

	   if(bUseTimer2Blink==0) // if NOT using Timer2 IRQ blink
	     {
	      if(GPIO_ReadOutputDataBit(LED_PORT, LED_PIN))
             GPIO_ResetBits(LED_PORT, LED_PIN); // LED1 OFF
	      else
		     GPIO_SetBits(LED_PORT, LED_PIN); // LED1 ON
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
  USART_PutStr(" 1. Start Timer2\r\n");
  USART_PutStr(" 2. Stop Timer2\r\n");
  USART_PutStr(" 3. Show Timer2 Counts\r\n");
  USART_PutStr(" 4. Toggle Timer2 IRQ On/Off\r\n");
  USART_PutStr(" B. Blink LED\r\n");
  USART_PutStr(" M. Re-Display Menu \r\n");
}

void processMenuCmd(char cmd)
{
  int x;
  uint16_t timer2Ctr;

  switch(cmd)
  {
   case '1':
	   USART_PutStr("\r\n  Start Timer2\r\n");
	   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); /* TIM2 clock enable */
	   TIM_Cmd(TIM2, ENABLE);  //Start the timer
   break;
   case '2':
	   USART_PutStr("\r\n  Stop Timer2\r\n");
	   TIM_Cmd(TIM2, DISABLE);
   break;
   case '3':
	   timer2Ctr = TIM_GetCounter(TIM2);
	   USART_PutStr("\r\n  TIMER2 = ");
	   USART_PutHexByte((uint8_t)(timer2Ctr >> 8));
	   USART_PutHexByte((uint8_t)timer2Ctr & 0x00FF);
	   USART_PutStr("   TIMER2 MS Counter = ");
	   USART_PutHexByte((uint8_t)(tm2MsCounter >> 8));
	   USART_PutHexByte((uint8_t)tm2MsCounter & 0x00FF);
	   USART_PutStr("\r\n");
   break;
   case '4':
	   USART_PutStr("\r\n  TIMER2 Interrupt is ");
	   if(bUseTimer2Blink==0)
	     {
		  setupTIM2();
		  USART_PutStr("ON\r\n");
		  bUseTimer2Blink=1;
	     }
	   else
	     {
		  TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
  	      TIM_Cmd(TIM2, DISABLE);
		  USART_PutStr("OFF\r\n");
		  bUseTimer2Blink=0;
	     }
   break;

   case 'B':
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
// setupTIM2()
// Init TM2 to generate interrupt every 1ms
// ***********************************************************************
void setupTIM2(void)
{
 TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
 TIM_OCInitTypeDef  TIM_OCInitStructure;
 NVIC_InitTypeDef NVIC_InitStructure;

 /* TIM2 clock enable */
 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

 // Timer2  Time base configuration - System clock is 72MHz
 TIM_TimeBaseStructure.TIM_Period = 1000 - 1;  // 1 MHz down to 1 KHz (1 ms)
 TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1; // 72 MHz Clock down to 1 MHz (adjust per your clock)
 TIM_TimeBaseStructure.TIM_ClockDivision = 0;
 TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

 TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

 // ******* Enable the TIM2 global Interrupt  **********
 NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

 NVIC_Init(&NVIC_InitStructure);

 /* TIM2 Interrupt enable */
 TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

 /* TIM2 enable counter */
 TIM_Cmd(TIM2, ENABLE);
}

// *******************************************************************************
// Function Name  : TIM2_IRQHandler
// Description    : This function handles TIM2 global interrupt request every 1ms
// *******************************************************************************
void TIM2_IRQHandler(void)
{
 if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
   {
    tm2MsCounter++;   //Just a demo counter


    if(++blinkCounter > 100)  // Blink LED fast (5 blinks/sec)
      {
    	blinkCounter=0;

    	if(GPIO_ReadOutputDataBit(LED_PORT, LED_PIN))
   		  GPIO_ResetBits(LED_PORT, LED_PIN);   // LED1 OFF
    	else
    	  GPIO_SetBits(LED_PORT, LED_PIN);    // LED1 ON
      }

	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
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

 // Enable USART1 and GPIOA clock
 RCC_APB2PeriphClockCmd (RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
 RCC_APB2PeriphClockCmd (RCC_APB2Periph_TIM1, ENABLE);

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



