// ***********************************************************************
// STM32F103_USARTx  - Template for USART Menu with GPIO & SysTick support
// Using USART1 PA10=RX PA9=TX
// SysTick interrupts every 10us
// Configure and read ADC_Channel_9 (PB1)for single read mode
//
// ** Use this project as a template for other work ***
// By Eric M. Klaus   10/2019
// ***********************************************************************
#include "stdio.h"
#include "stdlib.h"  //for itoa(), atoi()
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_adc.h"


#define LED_PIN   GPIO_Pin_13
#define LED_PIN2  GPIO_Pin_14
#define LED_PORT  GPIOC

/***************************************************************************//**
 * Declare function prototypes
 ******************************************************************************/
void showMenu(void);
void processMenuCmd(char cmd);
void ADC1_Config(void);

// SysTick based delay routines
void SysTick_Handler(void);

// Looping style delay routines (still controlled by SysTick)
//    NOTE: provided as examples, none used in this example
void Delay_us(const uint32_t usec);
void Delay_ms(const uint32_t msec);

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
 GPIO_InitStruct.GPIO_Pin = LED_PIN | LED_PIN2; // Configure Led pins
 GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
 GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;// Set Output Push-Pull
 GPIO_Init(LED_PORT, &GPIO_InitStruct);

 SetupUSART();

 GPIO_ResetBits(LED_PORT, LED_PIN);  //LED ON
 TimingDelay = 50000;                // Blink every 1/4 sec.

 showMenu();  // Display the user menu

 while(1)
   {
    if(TimingDelay==0)
	  {
	   TimingDelay = 50000;  //Reset the 1/4 sec. delay
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
  USART_PutStr(" 1. Option One \r\n");
  USART_PutStr(" 2. Option Two \r\n");
  USART_PutStr(" 3. Read ADC 9 \r\n");
  USART_PutStr(" B. Blink LED1 \r\n");
  USART_PutStr(" M. Re-Display Menu \r\n");
}

void processMenuCmd(char cmd)
{
  int x;
  switch(cmd)
  {
   case '1':
	   USART_PutStr("\r\n  You pressed ONE \r\n");
   break;
   case '2':
	   USART_PutStr("\r\n  You pressed TWO !! \r\n");
   break;
   case '3':
	   USART_PutStr("\r\n  Reading ADC 9..");
	   ADC1_Config();
	   USART_PutStr(" Init.. ");

	   ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_55Cycles5);

	   ADC_GetConversionValue(ADC1);  //Discard 1 reading - reset flags
	   ADC_SoftwareStartConvCmd(ADC1, ENABLE);

	   USART_PutStr(" Start.. ");
	   while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET){};  // wait for conversion to complete...

	   x = ADC_GetConversionValue(ADC1);   //Read the ADC value

	   itoa(x, msgBuf, 10);         // Convert result to printable string in msgBuf
	   USART_PutStr(" Result: ");   // Print the result
	   USART_PutStr(msgBuf);
	   USART_PutStr("\r\n");
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
// ADC1_Config()
// Configure ADC_Channel_9 (PB1)for single read mode
// ***********************************************************************
void ADC1_Config(void)
{
 ADC_InitTypeDef ADC_InitStructure;

 // Configure PB1 (ADC 9) as analog input
 GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
 GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
 GPIO_Init(GPIOB, &GPIO_InitStruct);

 //clock for ADC (max 14MHz --> 72/6=12MHz)
 RCC_ADCCLKConfig (RCC_PCLK2_Div6);

 /* Enable ADC1 & GPIOB clock */
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 |  RCC_APB2Periph_GPIOB, ENABLE);

 /* ADC1 configuration ------------------------------------------------------*/
 ADC_DeInit(ADC1);
 ADC_StructInit(&ADC_InitStructure);
 ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
 ADC_InitStructure.ADC_ScanConvMode = 0;       //Single read
 ADC_InitStructure.ADC_ContinuousConvMode = 0;  //Single read
 ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
 ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
 ADC_InitStructure.ADC_NbrOfChannel = 1;
 ADC_Init(ADC1, &ADC_InitStructure);  // ADC1 regular channel 9 configuration

 ADC_Cmd(ADC1, ENABLE);  //  Enable ADC1

 // Calibrate using internal REF Voltage
 ADC_TempSensorVrefintCmd(ENABLE);  // Need this to read Internal REF Voltage (ADC_Channel_Vrefint)
 ADC_ResetCalibration(ADC1);        // reset calibration register
 while (ADC_GetResetCalibrationStatus(ADC1)){}; // Wait for calibration to reset complete

 ADC_StartCalibration(ADC1);  // Start calibration
 while (ADC_GetCalibrationStatus(ADC1)){};  // Wait for calibration to complete

 ADC_SoftwareStartConvCmd(ADC1, ENABLE); // Start ADC1 Software Conversion
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



