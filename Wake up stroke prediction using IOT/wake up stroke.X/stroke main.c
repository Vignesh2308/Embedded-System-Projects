#define _XTAL_FREQ 20000000     /* define _XTAL_FREQ for using internal delay */
// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

#define buzz RB0

#define sw3 RB7
#define sw2 RB6
#define TEMP 1
#define HEART 2
#define RESP 3
#define LAT 4
#define LON 5
#define READY 6
#define Baud_rate 9600

#include <stdio.h>
#include <xc.h>
#include <stdlib.h>
#include <lcd.h>

unsigned int count = 0,cbeat = 0,pbeat = 0,temp,press,c_press_count=0,p_press_count=0,upload = 0;

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
inline unsigned char uart_response(void){
    unsigned char so_far[6] = {0,0,0,0,0,0};
    unsigned const char lengths[6] = {5,3,5,4,4,5};
    unsigned const char* strings[6] = {"temp:", "hr:", "pres:", "lat:", "lon:", "READY"};
    unsigned const char responses[6] = {TEMP, HEART, RESP, LAT , LON , READY};
    unsigned char received;
    unsigned char response;
    char continue_loop = 1;
    while (continue_loop) 
    {
        received = uart_rx();
        for (unsigned char i = 0; i < 6; i++) 
        {
            if (strings[i][so_far[i]] == received) 
            {
                so_far[i]++;
                if (so_far[i] == lengths[i]) 
                {
                    response = responses[i];
                    continue_loop = 0;
                }
            } 
            else 
            {
                so_far[i] = 0;
            }
        }
    }
    return response;
}
bit value1(void) {
    return (uart_response() == TEMP);
}
bit value2(void) {
    return (uart_response() == HEART);
}
bit value3(void) {
    return (uart_response() == RESP);
}
bit value4(void) {
    return (uart_response() == LAT);
}
bit value5(void) {
    return (uart_response() == LON);
}
 bit ready(void) {
    return (uart_response() == READY);
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

void timer1_init(){
    TMR1H = 0x00; // Set initial value for the Timer1
    TMR1L = 0x00;
    PIR1bits.TMR1IF = 0; // Reset the TMR1IF flag bit
    T1CONbits.TMR1CS = 0; // Timer1 counts pulses from internal oscillator
    T1CONbits.T1CKPS0 = 1; // Assigned prescaler rate is 1:8
    T1CONbits.T1CKPS1 = 1;
    PIE1bits.TMR1IE = 1; // Enable interrupt on overflow
    T1CONbits.TMR1ON = 1; // Turn the Timer1 on   
}

void buzzer(){
    buzz = 1;
    __delay_ms(500);
    buzz = 0;
    __delay_ms(500);
    buzz = 1;
    __delay_ms(500);
    buzz = 0;
    __delay_ms(500);
}

void main(){
    TRISB6 = 1;
    TRISB7 = 1;
    TRISB0 = 0;
    RB0 = 0;
    TRISA =0xFF;
    TRISD = 0x00;
    timer1_init();
    OPTION_REG = 0b00101000;
    TMR0 = 0;
    INTCON = 0xC0; // Enable interrupt (bits GIE and PEIE)
    char t[10],p[10],h[10];
    Lcd_Init();
    uart_init();
    adc_init();
    Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("     WAKE UP    ");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String(" STROKE PREDICT ");
    __delay_ms(2000);
    Lcd_Clear();
    do{
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String(" Initializing...");
        Lcd_Set_Cursor(2,1);
        Lcd_Write_String("   IOT MODEM    ");
    }while(!ready());
    Lcd_Clear();
    while(1){
        temp = adc_read(0);
        press = adc_read(1);
        temp = (float)temp * 0.4887;
        if(temp>=35){
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String("HIGH T");
            buzzer();
        }
        else{
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String("T : ");
            Lcd_Set_Cursor(1,5);
            sprintf(t,"%d ",temp);
            Lcd_Write_String(t);
        }
        if(pbeat != cbeat){
            Lcd_Set_Cursor(1,10);
            Lcd_Write_String("H : ");
            Lcd_Set_Cursor(1,14);
            sprintf(h,"%d ",cbeat);
            Lcd_Write_String(h);
            pbeat = cbeat;
            if(cbeat >= 140)
                buzzer();
        }
        else{
            Lcd_Set_Cursor(1,10);
            Lcd_Write_String("H : ");
            Lcd_Set_Cursor(1,14);
            sprintf(h,"%d ",pbeat);
            Lcd_Write_String(h);
        }
        if(press > 25){
            c_press_count++;
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String("    ");
            Lcd_Set_Cursor(2,5);
            Lcd_Write_String("RR : ");
            Lcd_Set_Cursor(2,10);
            sprintf(p,"%d ",c_press_count);
            Lcd_Write_String(p);
        }
        else
        {
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String("    ");
            Lcd_Set_Cursor(2,5);
            Lcd_Write_String("RR : ");
            Lcd_Set_Cursor(2,10);
            sprintf(p,"%d ",c_press_count);
            Lcd_Write_String(p);
            c_press_count = 0;
        }
        if(sw3 == 0){
            __delay_ms(50);
            if(sw3 == 0){
                Lcd_Clear();
                Lcd_Set_Cursor(1,1);
                Lcd_Write_String("  Uploading...  ");
                uart_txc('~');
                while(!value1());
                Lcd_Set_Cursor(1,1);
                Lcd_Write_String(" sending temp...");
                __delay_ms(50);
                uart_txs(t);uart_txc(0x0D);uart_txc(0x0A);
                while(!value2());
                Lcd_Set_Cursor(1,1);
                Lcd_Write_String(" sending hr...  ");
                __delay_ms(50);
                uart_txs(h);uart_txc(0x0D);uart_txc(0x0A);
                while(!value3());
                Lcd_Set_Cursor(1,1);
                Lcd_Write_String(" sending pres...");
                __delay_ms(50);
                uart_txs(p);uart_txc(0x0D);uart_txc(0x0A);
                while(!value4());
                Lcd_Set_Cursor(1,1);
                Lcd_Write_String(" sending lat... ");
                __delay_ms(50);
                uart_txs("1301.51\r\n");
                while(!value5());
                Lcd_Set_Cursor(1,1);
                Lcd_Write_String(" sending lon... ");
                __delay_ms(50);
                uart_txs("08013.54\r\n");
                while(!ready());
                Lcd_Set_Cursor(1,1);
                Lcd_Write_String("    Uploaded    ");
                Lcd_Set_Cursor(2,1);
                Lcd_Write_String("                ");
                __delay_ms(1000);
                Lcd_Clear();
            }
        }
    }
}
static void interrupt T1(){
    if(TMR1IF){
        TMR1IF = 0;
        count++;
        if(count == 95)   // 95 value for 10sec 
        {
            cbeat = TMR0;
            cbeat = cbeat * 6;
            count = 0;
            TMR0 = 0;
        }
    }
}