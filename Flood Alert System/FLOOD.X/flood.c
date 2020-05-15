#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RLED is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#define _XTAL_FREQ 20000000

#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

#define sw7 RB7

#define BUZZ RC0

#define Baud_rate 9600

unsigned int i;
char r,string[50];

#include <xc.h>
#include <stdio.h>
#include <lcd.h>
#include "string.h"

unsigned int count = 0,cbeat = 0,pbeat = 0,temp,press,c_press_count=0,p_press_count=0;
//UART declaration
void uart_init(){
    TRISC6 = 0; // TX Pin set as output
    TRISC7 = 1; // RX Pin set as input    
    SPBRG = ((_XTAL_FREQ/16)/Baud_rate) - 1;
    BRGH  = 1;  // for high baud_rate
    SYNC  = 0;    // Asynchronous
    SPEN  = 1;    // Enable serial port pins
    TXEN  = 1;    // enable transmission
    CREN  = 1;    // enable reception
    //RCIE  = 1;
    //CIF  = 0;
}   
void uart_txc(char ch)  {
    while(!TXIF);  // hold the program till TX buffer is free
    TXREG = ch; //Load the transmitter buffer with the received value
    while(!TRMT);
}
void uart_txs(char *st)  {
    while(*st) //if there is a char    
        uart_txc(*st++); //process it as a byte data
}
char uart_rx()   {
    if(OERR) // check for Error 
    {
        CREN = 0; //If error -> Reset 
        CREN = 1; //If error -> Reset 
    }
    while(!RCIF);  // hold the program till RX buffer is free
    return RCREG; //receive the value and send it to main function
}

unsigned char uart_rxs(){
    int i;
    for(i=0;i<40;i++)
    {
        r = uart_rx();
        if( r != 0x0D && r != 0x0A && r != 0x02)
        {	
            string[i] = r;
        }
        else
        {
            return(string);
        }
    }
}

void serial_println(const char *buffer){
    uart_txs(buffer);
    uart_txc(0x0D); //Ascii values for '\r'
    uart_txc(0x0A); // Ascii values for '\n'
}
//GSM module declaration
int index = 0;
char message_buffer[50];

void Setup_messaging(void){
    serial_println("AT+CMGF=1");
    __delay_ms(2000);
    serial_println("AT+CNMI=2,2,0,0,0");
    __delay_ms(2000);
}
void Start_message(char *phone_number){
    uart_txs("AT+CMGS=\"");
    uart_txs(phone_number);
    uart_txs("\"");
    uart_txc(0x0D);//ASCII values for '\r'
    __delay_ms(2000);
}
void Type_message_content(char *Content){
    uart_txs(Content);
}
void Send_message(void){
    uart_txc(0x1A);//ASCII value for 'ctrl+z'
}
void call_number(char *phone_number)
{
    uart_txs("ATD");
    uart_txs(phone_number);
    serial_println(";");
}
void end_call()
{
    uart_txs("ATH");
}

//End of GSM module
extern char message_buffer[50]; // define this for receiving messages
char phone_number[20] = "9566856263"; // Give a mobile number 10 digit.
char phone_number1[20] = "8111008776"; // Give a mobile number 10 digit.

//ADC FUNCTIONS
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

void main(void){
    TRISD = 0x00;
    TRISB = 0;
    PORTB = 0;
    TRISCbits.TRISC0 = 0;
    BUZZ = 0;
    TRISA = 0xFF;
    unsigned int N1,N2;
    uart_init();
    adc_init();
    Lcd_Init();    //This sets the PWM frequency of PWM1
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("  FLOOD ALERT ");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String("  IN PARKING  ");
    __delay_ms(3000);
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String("Message setup ");
    Setup_messaging();
    Lcd_Clear();
    while(1){
        N1 = adc_read(0);
        N2 = adc_read(1);
        BUZZ = 0;
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String(" PARKING LOT 1  ");
        Lcd_Set_Cursor(2,1);
        Lcd_Write_String(" DRY CONDITION  ");
        if(N1 < 500) 
        {
            BUZZ = 1;
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String("  WATERY FLOOR  ");
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String("   WARNING !!!  ");
            __delay_ms(2000);
        }
        if(N2 >500 && N1 < 500)
        {
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String("  FLOOD ALERT   ");
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String("Entering number1");
            Start_message(phone_number); 
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String(" Message typing ");
            Type_message_content("FLOOD ALERT \r\nPARKING LOT 1");
            Send_message();
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String("  Message sent  ");
            __delay_ms(2000);
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String("Entering number2");
            Start_message(phone_number1); 
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String(" Message typing ");
            Type_message_content("FLOOD ALERT \r\nPARKING LOT 1");
            Send_message();
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String("  Message sent  ");
            __delay_ms(2000);
            call_number(phone_number);
            __delay_ms(5000);
            __delay_ms(5000);
        }
    }
}