#define _XTAL_FREQ 20000000

#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

#define sw3 RB7

#define Baud_rate 9600

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

unsigned int count = 0,a=0,b=0,temp,gas;
unsigned char t[10],g[10];
    
    
void uart_init(){    
    TRISC6 = 0;   // TX Pin set as output
    TRISC7 = 1;   // RX Pin set as input    
    SPBRG = ((_XTAL_FREQ/16)/Baud_rate) - 1;
    BRGH  = 1;    // for high baud_rate
    SYNC  = 0;    // Asynchronous
    SPEN  = 1;    // Enable serial port pins
    TXEN  = 1;    // enable transmission
    CREN  = 1;    // enable reception
}
void uart_txc(char ch)  {
    int i;
    while(!TXIF)
        continue;
    TXREG=ch;
    for(i=0;i<500;i++);
}
void uart_txs(char *st)
{
    while(*st) //if there is a char    
        uart_txc(*st++); //process it as a byte data
}
char uart_rx()   {
    if(OERR) // check for Error 
    {
        CREN = 0;   //If error -> Reset 
        CREN = 1;   //If error -> Reset 
    }
    while(!RCIF);   // hold the program till RX buffer is free
    return RCREG;   //receive the value and send it to main function
}
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
//END OF ADC FUNCTIONS
void timer1_init(){
    TMR1H = 0x00;          // Set initial value for the Timer1
    TMR1L = 0x00;
    PIR1bits.TMR1IF = 0;   // Reset the TMR1IF flag bit
    T1CONbits.TMR1CS = 0;  // Timer1 counts pulses from internal oscillator
    T1CONbits.T1CKPS0 = 1; // Assigned prescaler rate is 1:8
    T1CONbits.T1CKPS1 = 1;
    PIE1bits.TMR1IE = 1;   // Enable interrupt on overflow
    T1CONbits.TMR1ON = 1;  // Turn the Timer1 on   
}

void main(){
    TRISD = 0x00;
    TRISA = 0xFF;
    TRISB7 = 1;
    TRISA = 0xFF; //PORTA as input
    timer1_init();
    OPTION_REG = 0b00101000;
    TMR0 = 0;
    INTCON = 0xC0; // Enable interrupt (bits GIE and PEIE)
    Lcd_Write_String(" Hazardous gas  ");
    uart_init();
    Lcd_Init();
    adc_init();
    Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String(" Hazardous gas  ");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String("Detection by IOT");
    __delay_ms(3000);
    Lcd_Clear();
    while(1){
        temp = adc_read(0);
        gas = adc_read(1);
        temp = (float)temp * 0.4887;
        if(temp>=35){
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String("HIGH TEMP       ");
        }
        else{
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String("TEMP : ");
            Lcd_Set_Cursor(1,8);
            sprintf(t,"%d ",temp);
            Lcd_Write_String(t);
        }
        if(gas >= 350){
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String("HIGH METHANE    ");
        }
        else{
            Lcd_Set_Cursor(2,1);    
            Lcd_Write_String("Methane = ");
            Lcd_Set_Cursor(2,11);
            sprintf(g,"%d ",gas);
            Lcd_Write_String(g);
        }
        if(count == 90){
            uart_txc('*');
            __delay_ms(100);
            uart_txc('T');
            __delay_ms(100);
            uart_txc(':');
            __delay_ms(100);
            uart_txs(t);	
            __delay_ms(100);
            uart_txc('M');
            __delay_ms(100);
            uart_txc(':');
            __delay_ms(100);
            uart_txs(g);
            __delay_ms(100);
            uart_txc('#');
            __delay_ms(100);
        }
    }
}
static void interrupt T1()
{
    if(TMR1IF)
    {
        TMR1IF = 0;
        count++;
        if(count == 95)
        {
            count = 0;
            TMR0 = 0;
        }
    }
}