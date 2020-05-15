#pragma config FOSC = HS       // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = OFF       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#define M_p RB0
#define M_n RB1
#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7
#define _XTAL_FREQ 20000000
#define Baud_rate 9600

#include <xc.h>
#include <lcd.h>
#include <stdio.h>

unsigned char  a = 0, b = 0,i = 0,t1 = 0,t2 = 0,rh1 = 0,rh2 = 0,sum = 0,recv,t[10],hum[10],m[10];
unsigned int temp,moist,ADCvalue1;

void uart_init(){    
    TRISC6 = 0; // TX Pin set as output
    TRISC7 = 1; // RX Pin set as inpu
    SPBRG = ((_XTAL_FREQ/16)/Baud_rate) - 1;
    BRGH  = 1;  // for high baud_rate
    SYNC  = 0;    // Asynchronous
    SPEN  = 1;    // Enable serial port pins
    TXEN  = 1;    // enable transmission
    CREN  = 1;    // enable reception
    RCIE  = 1;
    RCIF  = 0;
}
void uart_txc(char ch)  {
    while(!TXIF);  // hold the program till TX buffer is free
    TXREG = ch; //Load the transmitter buffer with the received value
    while(!TRMT);
}
void uart_txs(char *st)  {
    while(*st ) //if there is a char    
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

void StartSignal(){
    TRISB2 = 0;    //Configure RD2 as output
    RB2 = 0;    //RD2 sends 0 to the sensor
    __delay_ms(18);
    RB2 = 1;    //RD2 sends 1 to the sensor
    __delay_us(30);
    TRISB2 = 1;    //Configure RD2 as input
    }
void CheckResponse(){
    a = 0;
    __delay_us(40);
    if (RB2 == 0){
        __delay_us(80);
        if (RB2 == 1)
            a = 1;
        __delay_us(40);
    }
}
void ReadData(){
    for(b=0;b<8;b++){
        while(!RB2);
        __delay_us(30);
        if(RB2 == 0)    
            i&=~(1<<(7-b));  
        else{i|= (1<<(7-b));
        while(RB2);
        }  
    }
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

void main(){
    TRISD = 0x00;
    TRISA = 0xFF;
    TRISBbits.TRISB0 = 0;
    TRISBbits.TRISB1 = 0;
    M_p = 0;
    M_n = 0;
    Lcd_Init();
    uart_init();                          // uart Initialization
    adc_init();
    GIE  = 1;                             // Globel interrupt enable bit
    PEIE = 1;                             // Peripheral interrupt enable bit
    Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("LIFI Based Water");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String(" Dripping agri  ");
    __delay_ms(2000);
    Lcd_Clear();
    while(1){
        temp = adc_read(0);
        ADCvalue1 = adc_read(1);
        temp = temp*0.488;
        sprintf(t,"Tem:%uC ",temp);
        moist = ((float)ADCvalue1/1023)*100;
        StartSignal();
        CheckResponse();
        if(a == 1){
            ReadData();
            rh1 =i;
            ReadData();
            rh2 =i;
            ReadData();
            t1 =i;
            ReadData();
            t2 =i;
            ReadData();
            sum = i;
        }
        if(temp>=35){
            uart_txc('1');
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String("High tem");
        }
        else {
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String(t);
        }
        if(rh1>=60){
            uart_txc('1');
            Lcd_Set_Cursor(1,10);
            Lcd_Write_String(" Dry     ");
        }
        else {
            if(sum == rh1+rh2+t1+t2){
                Lcd_Set_Cursor(1,10);
                sprintf(hum,"RH:%u%% ",rh1);
                Lcd_Write_String(hum); 
            }
        }
        if(moist <= 50)
        {
            uart_txc('1');
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String("   Dry Soil   ");
        }
        else{
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String("   Wet Soil   ");
        }
        if(temp<40 && moist > 50 && rh1 < 60){
            uart_txc('0');
        }
    }
}

void interrupt ISR(){
    if(RCIF){
        RCIF = 0;
        recv = uart_rx();
        switch(recv){
            case '0':
                M_p = 0;
                M_n = 0;
                break;
            case '1':
                M_p = 1;
                M_n = 0;
                break;
            default:
                M_p = 0;
                M_n = 0;
                break;
        }
    }
}