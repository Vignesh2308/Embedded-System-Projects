#pragma config FOSC = HS       // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = OFF       // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#define _XTAL_FREQ	20000000

//LCD bits declaration
#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

#define Baud_rate 9600

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <string.h>
#include <lcd.h>

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
char uart_ready(){
   return RCIF;
}
unsigned char receive_data = 0;
unsigned char i,j; 
unsigned char rfid[10];

char uart_rxs(){
    for(i=0;i<15;)
    {
        receive_data = uart_rx();
        if(receive_data !=0x0D && receive_data != 0x0A)
        {
            rfid[i] = receive_data;
            i++;
        }
        else
        {return (rfid);}
    }
    
}
 
void main(void) {
    unsigned char card1[] = "0004263126";
    unsigned char card2[] = "0004263388";
    rfid[10]='\0';
    TRISB = 0xF0;
    TRISD = 0;
    Lcd_Init();
    uart_init();    //Initialize UART module
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("      RFID      ");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String("    Attendence  ");
    __delay_ms(2000);
    Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("  Message setup ");
    Lcd_Clear();
    while(1){
        nRBPU=0;
        PORTB = 0;
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String("  SHOW ID CARD  ");
        Lcd_Set_Cursor(2,1);
        Lcd_Write_String("                ");
        if(uart_rxs() != '\0')
        {
        Lcd_Clear();
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String("   ID   NUMBER  ");
        Lcd_Set_Cursor(2,4);
        Lcd_Write_String(rfid);
        __delay_ms(2000);
        i=1;
        
        if(strcmp(rfid,card1)==0){
            while(i){
                RB0 = 1;
                RB1 = 0;
                __delay_ms(250);
                Lcd_Clear();
                Lcd_Set_Cursor(1,1);
                Lcd_Write_String("   Student 1   ");
                Lcd_Set_Cursor(2,1);
                Lcd_Write_String("    Present    ");
                rfid[10]='\0';
                RB0 = 0;
                RB1 = 0;
                while(RB7);
                i=0;
            }
        }
            else if(strcmp(rfid,card2)==0){
                while(i){
                RB0 = 0;
                RB1 = 1;
                __delay_ms(250);
                Lcd_Clear();
                Lcd_Set_Cursor(1,1);
                Lcd_Write_String("   Student 2   ");
                Lcd_Set_Cursor(2,1);
                Lcd_Write_String("    Present    ");
                rfid[10]='\0';
                RB0 = 0;
                RB1 = 0;
                while(RB7);
                i=0;
                }
            }
        }
    }
}