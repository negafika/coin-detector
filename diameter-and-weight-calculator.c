/*
 * SEMESTER_PROJECT.c
 *
 * Created: 2/23/2020 6:53:54 AM
 * Author : Negaye and Abraham
 */ 


#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "math.h"
#include "atmega_lcd.h"

#define BAUD 9600
#define BRC  ((F_CPU/16/BAUD)-1)
#define BIT_IS_SET(byte, bit) (byte & (1 << bit))
#define BIT_IS_CLEAR(byte, bit) (!(byte & (1 << bit)))

void interrupt_init(void);
void timer_init(void);
void ADC_init(void);
void counter(void);
void EEPROM_write(int,int);
void Write_int_value(unsigned int);
void Display_Number_Lcd(char,char,int);
int EEPROM_read(int);

int piezo_read(void);
double Time_to_Length_Calculator(unsigned int,double);
double ChordLengths_to_Diameter(double,double);

double const g=9.8;
double const h1=0.13;
double const h2=0.16;
double const d=0.01;
double diameter;
unsigned int weight;
unsigned int timechange1,timechange2;
uint8_t swit;

int k=0;
int j=0;



int error_check=0;
int data_written=0;
int t[4]={0,0,0,0};

int second_counter=0;
int minute_counter=0;
int hour_counter=0;

unsigned int ndec,dec,ones,tens,lela,hulet;
double len1,len2;
unsigned char msg[] =" mm";
unsigned char error_msg[] = "invalid coin";



unsigned char rem_msg[]="Time remain is:";
unsigned char ord_msg[]="Insert coin";
unsigned char saved_msg[]="Data saved";
unsigned char cleared_msg[]="Data cleared";



int main(void)
{	
	UBRR0H = (BRC << 8);
	UBRR0L = BRC;
	
	UCSR0B = (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
   
	DDRB|=(1<<PB1);
	DDRD &=~(1<<PD2);
	DDRD &=~(1<<PD3);
	DDRD &=~(1<<PD4);
	DDRD &=~(1<<PD5);
	DDRD &=~(1<<PD6);
	DDRD &=~(1<<PD7);
	PORTD |= (1 << PD3);
	PORTD |= (1 << PD4);
	PORTD |= (1 << PD5);
	PORTD |= (1 << PD6);
	PORTD |= (1 << PD7);

	CLKPR = (1 << CLKPCE);
	CLKPR = 0x01;
	interrupt_init();
	timer_init();
	Lcd_Init();
	ADC_init();
	
    uint8_t timerOverflowCount=0;
    OCR0A =  0X7D;
	
	sei();
	TCNT0 = 0X00;
	
    while (1) 
    {

   
    swit = PIND & 0b00111000;
    if(timerOverflowCount >= 125){	
		 

		counter();
        
		if ((second_counter>0) | (minute_counter>0) | (hour_counter>0))
		{
			Lcd_Cmd(FIRST_ROW);
			Lcd_Write_Text(rem_msg);
			Display_Number_Lcd(2,14,second_counter);
			Lcd_Cmd(0xC0+11);
			Lcd_Write(':');
			Display_Number_Lcd(2,9,minute_counter);
			Lcd_Cmd(0xC0+6);
			Lcd_Write(':');
			Display_Number_Lcd(2,4,hour_counter);
		}
		else if ((second_counter==0) & (minute_counter==0) & (hour_counter==0))
		{
			PORTB &=~(1<<PB1);
			Lcd_Cmd(LCD_CLEAR);
			Lcd_Cmd(FIRST_ROW);
            Lcd_Write_Text(ord_msg);
		
	}		 
	if ( BIT_IS_CLEAR(PIND,PD7) & ( (EEPROM_read(3*(swit/8)) > 0) | (EEPROM_read(3*(swit/8)+1) > 0) | (EEPROM_read(3*(swit/8)+2) > 0)) )
	{
		EEPROM_write(3*(swit/8),0);
		EEPROM_write(3*(swit/8)+1,0);
		EEPROM_write(3*(swit/8)+2,0);
		Lcd_Cmd(LCD_CLEAR);
		Lcd_Write_Text(cleared_msg);
	}
	if (data_written==1)
	{
		Lcd_Cmd(LCD_CLEAR);
		Lcd_Write_Text(saved_msg);
		data_written = 0;
	}
	if (error_check==1)
	{
		Lcd_Cmd(LCD_CLEAR);
		Lcd_Write_Text(error_msg);
		error_check = 0;
	}
		
		timerOverflowCount = 0;
		
		}

        while((TIFR0 & (1<<OCF0A)) == 0);
		timerOverflowCount++;
	    TIFR0 |= (1 << OCF0A);
    }
	
}
void interrupt_init(void){
	EICRA=0b00000011;
	EIMSK=0b00000001;

}

void timer_init(void){
	TCCR0A = 0b00000010;
	TCCR0B = 0b00000100;
	TIMSK0 = 0b00000000;
	
	TCCR1A = 0b00000000;
	TCCR1B = 0b01000001;
	TIMSK1 = 0b00100000;
}

void ADC_init(void){
	ADMUX = 0b01100101;
	ADCSRA = 0b10000101;
	ADCSRB = 0b00000000;

}
int piezo_read(void)
{
	int a=0;
	int b=0;
	while(a<=b){
		a=b;
		ADCSRA |= (1 << ADSC);					// start ADC conversion
		while(BIT_IS_SET(ADCSRA, ADSC));
		b=ADCH;
		
	}
	return b;
}
void counter(void)
{
	

	if ((hour_counter>0) | (minute_counter>0) | (second_counter>0))
	{
		if (second_counter>0)
		{
			second_counter--;
		}
		else if(second_counter==0)
		{
			if (minute_counter>0)
			{
				minute_counter--;
				second_counter=59;
			}
			else if(minute_counter==0)
			{
				hour_counter--;
				minute_counter=59;
				second_counter=59;
			}
		}
		
		else if (minute_counter>0)
		{
			minute_counter--;
		}
		else if(minute_counter==0)
		{
			hour_counter--;
			minute_counter=59;
		}
		
	}
	else if (hour_counter==0)
	{
		if (second_counter>0)
		{
			second_counter--;
		}
		else if(second_counter==0)
		{
			if (minute_counter>0)
			{
				minute_counter--;
				second_counter=59;
			}
			else if(minute_counter==0)
			{
				second_counter=0;
			}
			
		}
		
	}
	
}


void Write_int_value(unsigned int value){
	
	int numOfDigits=log10(value);
	unsigned char*tempo=calloc(numOfDigits,sizeof(tempo));
	for(int i=0;i<=numOfDigits;i++)
	{
		
		tempo[numOfDigits-i]=(value%10)|0x30;
		value/=10;
	}
	Lcd_Write_Text(tempo);
}

void EEPROM_write(int uiAdress,int ucData)
{
	while(EECR & (1<<EEPE));
	EEAR=uiAdress;
	EEDR=ucData;
	EECR |=(1<<EEMPE);
	EECR |=(1<<EEPE);

}

int EEPROM_read(int uiAdress)
{
	while(EECR & (1<<EEPE));
	EEAR=uiAdress;
	EECR |=(1<<EERE);
	return EEDR;

}

void Display_Number_Lcd(char row,char col,int number)
{

	
	char tens, ones;
	int temp;
	
	temp = number;
	ones = temp % 10;
	temp = temp / 10;
	tens = temp % 10;

	
	if(row == 1)
	{
		Lcd_Cmd(0X80+col-1);
	}
	if(row == 2)
	{
		Lcd_Cmd(0XC0+col-1);
	}
	Lcd_Write(tens|0x30);
	Lcd_Write(ones|0x30);

}
double Time_to_Length_Calculator(unsigned int timechange,double h)
{
	double length,base;
	base = ((timechange/(4*pow(10,6)))/sqrt(2/g))+sqrt(h);
	length = base*base-h;
	return length;
}

double ChordLengths_to_Diameter(double b,double c)
{
	double y1=(0.25*(b*b-c*c)-d*d)/(2*d);
	double dia=sqrt(b*b+4*y1*y1);
	return dia;
}
	ISR(TIMER1_CAPT_vect){

		

		t[k]=ICR1;
		k++;
		
		if (k>3)
		{
			if (t[1]<t[0])
			{
				timechange1=(t[1]+65535)-t[0];
			}
			else{
				timechange1=t[1]-t[0];
			}
			if (t[3]<t[2])
			{
				timechange2=(t[3]+65535)-t[2];
			}
			else{
				timechange2=t[3]-t[2];
			}



		
			
		}
		if (TCCR1B == 0x41)
		{
			TCCR1B = 0x01;
		}
		else if (TCCR1B == 0x01)
		{
			TCCR1B = 0x41;
		}
		else{}
	}

ISR(INT0_vect){

      weight = piezo_read();
        
        len1=Time_to_Length_Calculator(timechange1,h1);
		len2=Time_to_Length_Calculator(timechange2,h2);
		diameter=ChordLengths_to_Diameter(len1,len2);
		diameter*=100000;
		ndec=diameter/100;
		ndec*=100;
		dec=diameter-ndec;
		ndec/=100;
		diameter=ndec+0.01*dec;
		
		if (BIT_IS_SET(PIND,PD6)){
			
			if (k>0)
			{

				for(int i=0;i<8;i++)
				{
					
					if ( (diameter>EEPROM_read(3*i) + EEPROM_read(3*i+1)*0.01-0.1) &  (diameter<EEPROM_read(3*i) + EEPROM_read(3*i+1)*0.01+0.1)  )
					{
						if ( (weight>EEPROM_read(3*i+2)-1) & (weight<EEPROM_read(3*i+2)+1))
						{

							minute_counter+=5*i+1;
							PORTB|=(1<<PB1);
							j=1;
						}
						
					}

				}
								if (j==0)
								{
		                          error_check = 1;
							    }
								else
								{
									j=0;
								}
			}
				

			

			else if (k==0)
			{
              error_check = 1;		
			}

		}
		
		
		else if (BIT_IS_CLEAR(PIND,PD6)){
			
			
			for(int i=0;i<8;i++)
			{
				
				if ((swit/8)==i)
				{
					EEPROM_write(3*i,ndec);
					EEPROM_write(3*i+1,dec);
					EEPROM_write(3*i+2,weight);
				    data_written = 1;
					
				}

			}

		}
								k=0;
								t[0]=0;
								t[1]=0;
								t[2]=0;
								t[3]=0;
								timechange1=0;
								timechange2=0;

			
				
	}

