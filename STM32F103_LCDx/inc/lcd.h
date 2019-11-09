// ***********************************************************************
// *	LCD interface header file
// ***********************************************************************

#define	LCD_RS GPIO_Pin_4
#define	LCD_RW GPIO_Pin_5
#define LCD_EN GPIO_Pin_6

#define	LCD_D4 GPIO_Pin_0
#define	LCD_D5 GPIO_Pin_1
#define LCD_D6 GPIO_Pin_2
#define LCD_D7 GPIO_Pin_3
#define LCD_DATA_PINS	LCD_D4 | LCD_D5 | LCD_D6 | LCD_D7
#define LCD_DATA_PORT   GPIOA
#define LCD_CTL_PORT    GPIOA

// Toggel the EN line Low to High then Low
void toggle_en(void);

// Write low 4bits to LCD
void lcd_write_4bits(unsigned char b);

// write a byte to the LCD in 4 bit mode 
extern void lcd_write(unsigned char);

// read a byte from the LCD in 4 bit mode
uint8_t lcd_read(char rs);

// Return non-zero if busy;
uint8_t lcd_busy(void);

// Clear and home the LCD 
extern void lcd_clear(void);

// write a string of characters to the LCD 
extern void lcd_puts(const char * s);

// Go to the specified position 
// Pos 0-0x0F = Line 1  0x40-0x4F=Line 2
extern void lcd_goto(unsigned char pos);
	
// intialize the LCD - call before anything else 
extern void lcd_init(void);

// Write a byte to the LCD display in 4 bit mode
extern void lcd_putch(char);

//	Set the cursor position 
#define	lcd_cursor(x)	lcd_write(((x)&0x7F)|0x80)
