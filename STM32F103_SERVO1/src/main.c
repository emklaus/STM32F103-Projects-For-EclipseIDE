// ***********************************************************************
// STM32F103_SERVO1  - Based on Template for USART Menu with GPIO & SysTick support
// Using USART1 PA10=RX PA9=TX with SysTick interrupts every 10us
// Demo using TIMER2 to generate interrupt every 20ms and at servo timeout event (CC1).
//
// Connectiond: Servo Signal Input = PC14   Servo VCC=+5V  Servo GND = GND
// NOTE: Servo needs to be powered by +5V NOT 3.3V
//
// ** Used this project as a template for other work ***
// By Eric M. Klaus   10/2019
// ***********************************************************************
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"
#include "stdlib.h"  //for itoa(), atoi()

#define LED_PIN  GPIO_Pin_13
#define LED_PORT GPIOC

#define SERVO1_PIN  GPIO_Pin_14
#define SERVO1_PORT GPIOC

#define TIMER_VAL_20MS 20000
// ** Typically, SERVO_MIN_COUNTS would be 1000 and SERVO_MAX_COUNTS would be 2000 **
// ** I found my servos (HS-322HD, HS425BB) responded differently - adjust accordingly ***
#define SERVO_MIN_COUNTS 650
#define SERVO_MAX_COUNTS 2450
#define SERVO_RANGE_COUNTS (SERVO_MAX_COUNTS - SERVO_MIN_COUNTS)
#define SERVO_MID_COUNTS  SERVO_MIN_COUNTS + (SERVO_RANGE_COUNTS/2)

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
static __IO uint32_t servo1_Pos = SERVO_MID_COUNTS;

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


 // Configure the GPIO pin for SERVO1 (PC14)
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); // Clock PORTC Enable
  GPIO_InitStruct.GPIO_Pin = SERVO1_PIN;                // Configure Led pin
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;         // Set pin as Output Push-Pull
  GPIO_Init(SERVO1_PORT, &GPIO_InitStruct);
  GPIO_ResetBits(SERVO1_PORT, SERVO1_PIN); // Servo signal LOW


 SetupUSART();   // Configure USART1

 GPIO_ResetBits(LED_PORT, LED_PIN);   // LED ON
 TimingDelay = 50000;                 // Blink every 1/2 sec.

 showMenu();  // Display the user menu

 while(1)
   {
    if(TimingDelay==0)
	  {
	   TimingDelay = 50000;  //Reset the 1/2 sec. delay driven by SysTick

       if(GPIO_ReadOutputDataBit(LED_PORT, LED_PIN))
          GPIO_ResetBits(LED_PORT, LED_PIN); // LED1 OFF
       else
	      GPIO_SetBits(LED_PORT, LED_PIN); // LED1 ON

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
  USART_PutStr(" 3. Set Servo Position\r\n");
  USART_PutStr(" 4. Set Servo Counts\r\n");
  USART_PutStr(" B. Blink LED\r\n");
  USART_PutStr(" M. Re-Display Menu \r\n");
}

void processMenuCmd(char cmd)
{
  int x, pos;
  uint16_t timer2Ctr;

  switch(cmd)
  {
   case '1':
	   USART_PutStr("\r\n  Start Timer2\r\n");
	   setupTIM2();
   break;
   case '2':
	   USART_PutStr("\r\n  Stop Timer2\r\n");
	   TIM_Cmd(TIM2, DISABLE);
   break;
   case '3':
	   USART_PutStr("\r\n  Input Servo Pos [0-180]: ");
	   USART_GetStr((char *)msgBuf, 3);
	   pos = atoi((char *)msgBuf);
	   servo1_Pos =(uint32_t)(float)pos * ((float)SERVO_RANGE_COUNTS/180)+SERVO_MIN_COUNTS;
	   USART_PutStr("\n OK\r\n");
   break;
   case '4':
	   USART_PutStr("\r\n  Input Servo Counts [1000-2000]: ");
	   USART_GetStr((char *)msgBuf, 4);
	   servo1_Pos = atoi((char *)msgBuf);
	   USART_PutStr("\n OK\r\n");
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

//***************************************************************************
// setupTIM2(void)
// Configure and start Timer2 counting @ 1us/count
// It will generate a TIM_IT_Update interrupt at 20ms
// and a TIM_IT_CC1 interrupt each time the Compare1 count is reached
//***************************************************************************
void setupTIM2(void)
{
  // Timer & Interrupt Controller Init structures
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  NVIC_InitTypeDef         NVIC_InitStructure;

  // ******* Enable the TIM2 gloabal Interrupt  **********
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

  NVIC_Init(&NVIC_InitStructure);

  /* TIM2 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);


  // Timer2  Time base configuration - System clock is 72MHz
  TIM_TimeBaseStructure.TIM_Period = TIMER_VAL_20MS - 1; // TIM_IT_Update will occur every 20ms
  TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;          // 72 MHz Clock down to 1 MHz (adjust per your clock)
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  TIM_SetCompare1(TIM2, servo1_Pos); // Set the compare 1 value to the servo position value

  /* TIM IT enable */
  TIM_ITConfig(TIM2, TIM_IT_Update | TIM_IT_CC1, ENABLE);

  /* TIM2 enable counter */
  TIM_Cmd(TIM2, ENABLE);
}


// *******************************************************************************
// Function Name  : TIM2_IRQHandler
// Description    : This function handles TIM2 global interrupt request.
//
// *******************************************************************************
void TIM2_IRQHandler(void)
{
  int x;
  uint16_t cc1Val;

  if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  // when the 20ms count is reached
    {
	 GPIO_SetBits(SERVO1_PORT, SERVO1_PIN);   // Servo signal High
	 TIM_SetCompare1(TIM2, servo1_Pos);       // Set the compare 1 value to the servo position value
	 TIM_ClearITPendingBit(TIM2, TIM_IT_Update);   // Clear the IRQ flag
    }

  if(TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET) // when the Compare1 count is reached
    {
	 GPIO_ResetBits(SERVO1_PORT, SERVO1_PIN); // Servo signal Low
	 TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);   // Clear the IRQ flag
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



