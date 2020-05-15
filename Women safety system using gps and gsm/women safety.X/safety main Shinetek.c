#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// DEFINITIONS
#define Baud_rate 9600
#define _XTAL_FREQ 20000000

#define OUTPUT 0
#define INPUT 1

#define SUCCESS 1
#define FAILURE 0
//Bits configuration
#define BIT0 0x01    
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7
#define sw RB5
#define buzzer RB4

#include <stdio.h>
#include <xc.h>
//#include <string.h>
//#include <stdbool.h>
#include <lcd.h>

void UART_Init(void)
{
    TRISC6 = 0; // TX Pin set as output
    TRISC7 = 1; // RX Pin set as input
    SPBRG = ((_XTAL_FREQ/16)/Baud_rate) - 1;
    BRGH  = 1;  // for high baud_rate
    SYNC  = 0;    // Asyn1chronous
    SPEN  = 1;    // Enable serial port pins
    TXEN  = 1;    // enable transmission
    CREN  = 1;    // enable reception
    TX9   = 0;    // 8-bit reception selected
    RX9   = 0;    // 8-bit reception mode selected
   }

void Serial_write(char data){   
    while(!(TXSTA & BIT1));
    //Now write the data to USART buffer
    TXREG=data;      
}
void Serial_print(const char *buffer){
    int i;
    for (i=0;buffer[i] != '\0' ;i++)
    Serial_write(buffer[i]);
}
void Serial_println(const char *buffer){
    Serial_print(buffer);
    Serial_write(0x0D); //Ascii values for '\r'
    Serial_write(0x0A); // Ascii values for '\n'
}
//GSM module declaration
int index = 0;
char message_buffer[50];

void Setup_messaging(void){
    Serial_println("AT+CMGF=1");
    __delay_ms(2000);
    Serial_println("AT+CNMI=2,2,0,0,0");
    __delay_ms(2000);
}
void Start_message(char *phone_number){
    Serial_print("AT+CMGS=\"");
    Serial_print(phone_number);
    Serial_print("\"");
    Serial_write(0x0D);//ASCII values for '\r'
    __delay_ms(2000);
}
void Type_message_content(char *Content){
    Serial_print(Content);
}
void Send_message(void){
    Serial_write(0x1A);//ASCII value for 'ctrl+z'
}
extern char message_buffer[50]; // define this for receiving messages
char phone_number1[20] = "7338990989"; // Give a mobile number 10 digit.
char phone_number2[20] = "7871034521";
char phone_number3[20] = "9600082342";

void main()
{
    TRISD = 0x00;
    TRISBbits.TRISB4 = 0;
    TRISBbits.TRISB5 = 1;
    Lcd_Init();
    UART_Init();
    Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("  Women safety  ");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String("  Alert system  ");
    __delay_ms(1000);
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("  Message setup ");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String("                ");
    Setup_messaging();
    Lcd_Clear();
    buzzer = 0;
while(1)
{   buzzer = 0;
    if(!sw)
    {
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String(" Getting GPS.   ");
        __delay_ms(500);
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String(" Getting GPS..  ");
        __delay_ms(500);
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String(" Getting GPS... ");
        Lcd_Set_Cursor(2,1);
        Lcd_Write_String("                ");
        __delay_ms(1000);
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String("LAT : 13.03 N  ");
        Lcd_Set_Cursor(2,1);
        Lcd_Write_String("LON : 80.13 E  ");
        Start_message(phone_number1);
        Type_message_content("EMERGENCY URGENT\rLAT : 13.03 N\rLON : 80.13 E \r\n");
        __delay_ms(1000);
        Send_message();
        Start_message(phone_number2);
        Type_message_content("EMERGENCY URGENT\rLAT : 13.03 N\rLON : 80.13 E \r\n");
        __delay_ms(1000);
        Send_message();
        Start_message(phone_number3);
        Type_message_content("EMERGENCY URGENT\rLAT : 13.03 N\rLON : 80.13 E \r\n");
        __delay_ms(1000);
        Send_message();
        Lcd_Clear();
        Lcd_Set_Cursor(1,1); 
        Lcd_Write_String("     HELP !!!   ");
        Lcd_Set_Cursor(2,1);
        Lcd_Write_String("                ");
        __delay_ms(2000);
        Lcd_Clear();
        buzzer = 1;
        __delay_ms(2000);
                }
    }

}