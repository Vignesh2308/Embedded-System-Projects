#define _XTAL_FREQ 20000000

#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

#define sw RB5
#define red RB1 
#define green RB2
#define MP RB3
#define MN RB4

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

#define SUCCESS 1
#define FAILURE 0

/****Uart baud rate values for 20 MHz crystal oscillator*****/
#define BAUD_9600    129
#define BAUD_19200   64
#define BAUD_38400   32
#define BAUD_57600   21
#define BAUD_115200  12

#pragma config FOSC = HS // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = OFF // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF // Flash Program Memory Code Protection bit (Code protection off)
#include <xc.h>
#include <stdio.h>
#include <lcd.h>

unsigned int count = 0,a=0,b=0,c=0;
unsigned char x[],y[],z[];


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
void Serial_begin(int baudrate){
  TRISC6 = 0;
  TRISC7 = 1;
  TXSTA = (BIT5|BIT2);  // Transmit enabled, high speed async mode
  SPBRG = baudrate;  //((char)(FOSC/ (16 * BAUD_RATE )) – 1)
  RCSTA = (BIT7|BIT4);
  RCIE=1;   // enable uart recieve interrupt
  PEIE=1;   // enable peripheral interrupts                                    
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
//End of UART 

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
void Call_number(char *phone_number)
{
    Serial_print("ATD=");
    Serial_print(phone_number);
    Serial_println(";");
}
//End of GSM module
extern char message_buffer[50]; // define this for receiving messages
char phone_number3[20] ="9600082342"; // Give a mobile number 10 digit.
char phone_number2[20] ="7548849937"; // Give a mobile number 10 digit.
char phone_number1[20] ="7010046623"; // Give a mobile number 10 digit.
void main(){
    TRISD = 0x00;
    TRISA = 0xFF;
    TRISB5 = 1;
    TRISB0 = 1;
    TRISB1 = 0;
    TRISB2 = 0;
    TRISB3 = 0;
    TRISB4 = 0;
    red = 0;green =0;MP = 0;MN = 0;
    Serial_begin(BAUD_9600);
    Lcd_Init();
    Lcd_Clear();
    adc_init();
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("Automob exhaust ");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String(" Monitoring sys ");
    __delay_ms(3000);
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("Message setup...");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String("                ");
    Setup_messaging();
    Lcd_Clear();
    while(1){
        Lcd_Set_Cursor(2,1);
        Lcd_Write_String("                ");
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String("   Turn ON KEY  ");
        red = 1;green =0;MP = 0;MN = 0;
        if(sw == 0){
            __delay_ms(50);
            if(sw == 0){
                Lcd_Set_Cursor(1,1);
                Lcd_Write_String(" Engine start...");
                Lcd_Set_Cursor(1,1);
                Lcd_Write_String("Checking exhaust");
                __delay_ms(2000);
                if(RB0 == 0){
                    red = 1;green =0;MP = 0;MN = 0;
                    Lcd_Set_Cursor(1,1);
                    Lcd_Write_String("Engine shutdown ");
                    Lcd_Set_Cursor(2,1);
                    Lcd_Write_String(" Presence of CO ");
                    __delay_ms(3000);
                    Start_message(phone_number1);
                    Lcd_Set_Cursor(1,1);
                    Lcd_Write_String("  Sending MSG1  ");
                    Type_message_content("High CO content in exhaust\r\nService your vehicle");
                    Send_message();
                    Lcd_Set_Cursor(1,1);
                    Lcd_Write_String("Message SENT");
                    __delay_ms(1000);
                    Start_message(phone_number2);
                    Lcd_Set_Cursor(1,1);
                    Lcd_Write_String("  Sending MSG2  ");
                    Type_message_content("High CO content in exhaust\r\nService your vehicle");
                    Send_message();
                    Lcd_Set_Cursor(1,1);
                    Lcd_Write_String("Message SENT");
                    __delay_ms(1000);
                    Start_message(phone_number3);
                    Lcd_Set_Cursor(1,1);
                    Lcd_Write_String("  Sending MSG3  ");
                    Type_message_content("High CO content in exhaust\r\nService your vehicle");
                    Send_message();
                    Lcd_Set_Cursor(1,1);
                    Lcd_Write_String("Message SENT");
                    __delay_ms(1000);
                    Lcd_Clear();
                }
                else{
                    red = 0;green =1;MP = 1;MN = 0;
                    Lcd_Set_Cursor(1,1);
                    Lcd_Write_String("   Engine ON    ");
                    Lcd_Set_Cursor(2,1);
                    Lcd_Write_String("  Absence of CO ");
                    __delay_ms(5000);
                    Lcd_Clear();
                }   
            }
        }
    }
}