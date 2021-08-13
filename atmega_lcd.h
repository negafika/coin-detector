/*
 * atmega_lcd.h
 *
 * Created: 12/7/2019 5:32:40 AM
 * Author : 
 */ 


#include<util/delay.h>



#define LCD_CLEAR		0x01
#define CURSOR_OFF 	    0x0C
#define FIRST_ROW		0x80
#define SECOND_ROW	    0xC0
#define Enable_Pulse()	PORTC|=1<<EN;_delay_ms(1);PORTC &=~(1<<EN);_delay_ms(1);

#define D0              PC0
#define D1              PC1
#define D2              PB2
#define D3              PB3
#define D4              PB4
#define D5              PB5
#define D6              PB6
#define D7              PB7


#define	RS			    PC2
#define	RW			    PC3
#define	EN			    PC4

void Lcd_Init(void);
void Lcd_Cmd(unsigned char value);
void Lcd_Write(unsigned char value);
void Lcd_Write_Text(unsigned char msg[]);
void Lcd_Data_Shift(unsigned char value);




void Lcd_Init(void)
{
	DDRC |= (1<<RS);					//RS Pin as Output Pin
	DDRC |= (1<<RW);					//RW Pin as Output Pin
	DDRC |= (1<<EN);					//EN Pin as Output Pin
   
  //  DDRC |=(1<<PC0);
    DDRB |= 0xFC;				//P0.8 to P0.15 as Data Line of LCD
	DDRC |=(1<<PC0);
	DDRC |=(1<<PC1);
	
	Lcd_Cmd(0x38);							//Send 8-bit initialization command to lcd
	_delay_ms(10);
	Lcd_Cmd(CURSOR_OFF);				//Cursor OFF
	_delay_ms(10);
	Lcd_Cmd(LCD_CLEAR);
	_delay_ms(1);
	Lcd_Cmd(FIRST_ROW);
}

void Lcd_Data_Shift(unsigned char value)
{
	/*
	This Function will shift the eight bit data stored in variable value,
	to the Port Pin P0.8 to P0.15 Successfully.
	*/
	unsigned char i;
	
	for(i=0;i<8;i++)
	{ 
	
		if(value & 0x01)
		{ 
			if (i<2)
			{
			PORTC |= (1<<i);
			} 
			else
			{
			PORTB |= (1<<i);
			}
		}
		else
		{
			if (i<2)
			{
			PORTC &= ~(1<<i);
			} 
			else
			{
			PORTB &= ~(1<<i);
			}
		}
		value = value >> 1;
	}
}

void Lcd_Cmd(unsigned char value)
{
	/*Configure LCD for receiving Command Data*/
	PORTC &= ~(1<<RS);
	PORTC &= ~(1<<RW);
	PORTC &= ~(1<<EN);
	Lcd_Data_Shift(value);
	Enable_Pulse();
}

void Lcd_Write(unsigned char value)
{
	/*Configure LCD for receiving Display Data*/
	PORTC |= (1<<RS);
	PORTC &= ~(1<<RW);
	PORTC |= (1<<EN);
	Lcd_Data_Shift(value);
	Enable_Pulse();
}

void Lcd_Write_Text(unsigned char msg[])
{
	while(*msg)
	{
		Lcd_Write(*msg);
		msg++;
	}
}

