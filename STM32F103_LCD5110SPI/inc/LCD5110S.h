// ***************************************************************
// LCD5110S.h
// General routines for controlling the NOKIA 5110 48x84 LCD
// Using the SPI Peripheral
// 
// PB10=Backlight (violet)    PB1=DC (orange)    PB0=CE (Blue)
// PA7=DIN (Green)            PA6=RST (Ora/Blk)  PA5=CLK (Yellow)
//
// This odd selection of GPIO pins are used only because I had a
// connector wired to my LCD module for use on the Arduino and
// was too lazy to make another one. since SCK & MOSI lined up
// I simply used software to make the other signals function properly
// a simpler approach of course would be to use PA2-PA7.
// ***************************************************************
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "spi.h"

#define PIN_SCLK  GPIO_Pin_5
#define PIN_RESET GPIO_Pin_6
#define PIN_SDIN  GPIO_Pin_7
#define PIN_SCE   GPIO_Pin_0
#define PIN_DC    GPIO_Pin_1
#define PIN_BKLT  GPIO_Pin_10

#define LCD_C     0    //LCD Command
#define LCD_D     1    //LCD Data

#define LCD_X     84   // Dot Cols
#define LCD_Y     48   // Dot rows

void LcdInitialize(void);      // Initialize   the LCD
void LcdCharacter(char character);  // Display character at the current output position
void LcdCharacterX2(char character, char row, char col); 
void LcdStringX2(char *str, char row, char col);
void LcdClear(void);
void LcdString(char *str);
void LcdWrite(uint8_t dc, uint8_t data);
void shiftOut8Bits(uint8_t data);  // For Bit-Bang Mode
void LcdBmp(const uint8_t *my_array);
void LCD_GotoXY(uint8_t x, uint8_t y );
void LcdHBar(uint8_t row, uint8_t start, uint8_t len, uint8_t bClear);
void us_delay(uint32_t usec);
void EnableCoreTiming(void);
void CoreTimingDelay(unsigned int tick);

