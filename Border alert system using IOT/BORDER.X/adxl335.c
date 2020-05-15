#define _XTAL_FREQ 20000000

#define RS RD7
#define EN RD6
#define D4 RD5
#define D5 RD4
#define D6 RD3
#define D7 RD2

#define led RB2

#pragma config FOSC = HS // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = OFF // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF // Flash Program Memory Code Protection bit (Code protection off)

#include <stdio.h>
#include <xc.h>
#include <stdlib.h>
#include <lcd.h>

unsigned int x,y,z;
char a[10],b[10],c[10];

void adc_init(){
  ADCON0 = 0x81;               //Turn ON ADC and Clock Selection
  ADCON1 = 0xC0;               //All pins as Analog Input and setting Reference Voltages
}
unsigned int adc_read(unsigned char channel){
  if(channel > 7)              //Channel range is 0 ~ 7
    return 0;

  ADCON0 &= 0xC5;              //Clearing channel selection bits
  ADCON0 |= channel<<3;        //Setting channel selection bits
  __delay_ms(2);               //Acquisition time to charge hold capacitor
  GO_nDONE = 1;                //Initializes A/D conversion
  while(GO_nDONE);             //Waiting for conversion to complete
  return ((ADRESH<<8)+ADRESL); //Return result
}

void main(){
    TRISD = 0x00;
    TRISA = 0xFF;
    TRISB2 = 0;
    led = 0;
    Lcd_Init();
    adc_init();
    Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("  BORDER ALERT  ");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String(" SYS USING IOT  ");
    __delay_ms(3000);
    Lcd_Clear();
    while(1){
        x = adc_read(0);
        Lcd_Set_Cursor(1,1);
        sprintf(a,"X=%d ",x);
        Lcd_Write_String(a);
        y = adc_read(1);
        Lcd_Set_Cursor(1,9);
        sprintf(b,"Y=%d ",y);
        Lcd_Write_String(b);
        z = adc_read(2);
        Lcd_Set_Cursor(2,1);
        sprintf(c,"Z=%d ",z);
        Lcd_Write_String(c);
        if(x <= 300 ){
           led = 1; 
        }
        else
        {
            led = 0;
        }
    }
}