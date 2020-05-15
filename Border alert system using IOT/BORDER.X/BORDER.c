#define _XTAL_FREQ 20000000

#define RS RD7
#define EN RD6
#define D4 RD5
#define D5 RD4
#define D6 RD3
#define D7 RD2

#define MP RB0
#define MN RB1

#define buzz RB2
#define led RB3
#define sw3 RB7

#define Baud_rate 9600

#pragma config FOSC = HS // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF // Watchdog Timer Enable bit (WDT enabbuzz)
#pragma config PWRTE = OFF // Power-up Timer Enable bit (PWRT disabbuzz)
#pragma config BOREN = OFF // Brown-out Reset Enable bit (BOR enabbuzz)
#pragma config LVP = OFF // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF // Flash Program Memory Code Protection bit (Code protection off)
#include <stdio.h>
#include <xc.h>
#include <stdlib.h>
#include <lcd.h>
#include "string.h"

unsigned int count = 0,CNT = 0,x,y,z;
unsigned char t[10],g[10];
unsigned int finish =0,pos_cnt=0,lat_cnt=0,log_cnt=0,flg=0,com_cnt=0,lat_dir_cnt=0,lon_dir_cnt=0;            
unsigned char lat[20],lon[20],Gpsdata,lat_dir[5],lon_dir[5];   

void uart_init(){    
    TRISC6 = 0;   // TX Pin set as output
    TRISC7 = 1;   // RX Pin set as input    
    SPBRG = ((_XTAL_FREQ/16)/Baud_rate) - 1;
    BRGH  = 1;    // for high baud_rate
    SYNC  = 0;    // Asynchronous
    SPEN  = 1;    // Enable serial port pins
    TXEN  = 1;    // enable transmission
    CREN  = 1;    // enable reception
    RCIE = 1;
    RCIF = 0;
}
void uart_txc(char ch)  {
    int i;
    while(!TXIF)
        continue;
    TXREG=ch;
    for(i=0;i<500;i++);
}
void uart_txs(char *st){
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
    TRISB6 = 1;
    TRISB0 = 0;
    TRISB1 = 0;
    TRISB2 = 0;
    TRISB3 = 0;
    buzz = 0;
    led = 0;
    MP = 0;
    MN = 0;
    timer1_init();
    OPTION_REG = 0b00101000;
    TMR0 = 0;
    INTCON = 0xC0; // Enable interrupt (bits GIE and PEIE)
    uart_init();
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
        y = adc_read(1);
        z = adc_read(2);
        MP = 1;
        MN = 0;
        buzz = 0;
        if(x <= 290){led = 1;}
        else{led = 0;}
        while(finish!=1);
        if(sw3 == 0){
            __delay_ms(100);
            if(sw3 == 0){
                CNT = 1;
                buzz = 1;
                MP = 0;
                MN = 0;
                Lcd_Set_Cursor(1,1);
                Lcd_Write_String("LAT:1306.6243N  ");   //13.066243, 80.966609
                Lcd_Set_Cursor(2,1);
                Lcd_Write_String("LON:08096.6609E ");
                __delay_ms(2000);
                Lcd_Set_Cursor(1,1);
                Lcd_Write_String("BORDER CROSSED  ");  
                Lcd_Set_Cursor(2,1);
                Lcd_Write_String("                ");
                __delay_ms(2000);
                buzz = 0;
                Lcd_Clear();
            }
        }
        else{
            buzz = 0;
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String("LAT:");    // printing GPS data to serial monitor
            Lcd_Set_Cursor(1,5);
            Lcd_Write_String(lat);
            Lcd_Write_String(lat_dir);
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String("LON:");
            Lcd_Set_Cursor(2,5);
            Lcd_Write_String(lon);
            Lcd_Write_String(lon_dir);
            MP = 1;
            MN = 0;
            finish = 0;
            pos_cnt = 0;
        }
    }
}

void interrupt ISR(){
    if(TMR1IF)
    {
        TMR1IF = 0;
        count++;
        if(count == 95)
        {
            if(CNT == 1){
                uart_txc('*');
                __delay_ms(100);
                uart_txs("CROSSED BORDER");
                __delay_ms(100);
                uart_txc('#');
                __delay_ms(100);
                CNT = 0;
            }
            else{
                uart_txc('*');
                __delay_ms(100);
                uart_txs("lat:");
                __delay_ms(100);
                uart_txs(lat);	
                __delay_ms(100);
                uart_txs(lat_dir);
                __delay_ms(100);
                uart_txc('_');
                __delay_ms(100);
                uart_txs("lon:");
                __delay_ms(100);
                uart_txs(lon);
                __delay_ms(100);
                uart_txs(lon_dir);
                __delay_ms(100);
                uart_txc('#');
                __delay_ms(100);
                count = 0;
                TMR0 = 0;
            }
        }
    }
    if(RCIF){
        RCIF = 0;
        Gpsdata = uart_rx();
        flg = 1;
        if(finish == 0){
            if( Gpsdata=='$' && pos_cnt == 0)  // finding GPRMC header
                pos_cnt=1;
            if( Gpsdata=='G' && pos_cnt == 1)
                pos_cnt=2;
            if( Gpsdata=='P' && pos_cnt == 2)
                pos_cnt=3;
            if( Gpsdata=='R' && pos_cnt == 3)
                pos_cnt=4;
            if( Gpsdata=='M' && pos_cnt == 4)
                pos_cnt=5;
            if(Gpsdata=='C'  &&  pos_cnt == 5 )
                pos_cnt=6;
            if(pos_cnt==6    &&  Gpsdata ==','){  // count commas in message
                com_cnt++;
                flg=0;
            }
            if(com_cnt==3 && flg==1){
                lat[lat_cnt++] =  Gpsdata;        // latitude
                flg=0;
            }
            if(com_cnt==4 && flg==1){
                lat_dir[lat_dir_cnt++] = Gpsdata;
            }
            if(com_cnt==5 && flg==1){
                lon[log_cnt++] =  Gpsdata;        // longitude
                flg=0;
            }
            if(com_cnt==6 && flg==1){
                lon_dir[lon_dir_cnt++] = Gpsdata;
            }
            if( Gpsdata == '*' && com_cnt >= 5 && flg == 1){
                lat[lat_cnt] ='\0';
                lon[log_cnt]  = '\0';
                lat_dir[lat_dir_cnt]='\0';
                lon_dir[lon_dir_cnt]='\0';
                lat_dir_cnt=0;
                lon_dir_cnt=0;
                lat_cnt = 0;
                log_cnt = 0;
                flg     = 0;
                finish  = 1;
                com_cnt = 0;
            }
        }
    }
}