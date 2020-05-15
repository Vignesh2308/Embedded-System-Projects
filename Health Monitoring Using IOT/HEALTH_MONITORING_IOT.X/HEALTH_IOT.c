#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#define _XTAL_FREQ 20000000

#define RS RD7
#define EN RD6
#define D4 RD5
#define D5 RD4
#define D6 RD3
#define D7 RD2

#define Baud_rate 9600

#include <xc.h>
#include <stdio.h>
#include <lcd.h>
#include <string.h>

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
    //RCIF  = 0;
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

//END OF ADC FUNCTIONS
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

void main(void){
    unsigned int temp,cpress = 0,i,press;
    char t[10],p[10],h[10];
    TRISD = 0x00;
    TRISA = 0xFF; //PORTA as input
    timer1_init();
    OPTION_REG = 0b00101000;
    TMR0 = 0;
    INTCON = 0xC0; // Enable interrupt (bits GIE and PEIE)
    uart_init();    
    adc_init();    //Initializes ADC Module
    Lcd_Init();    //This sets the PWM frequency of PWM1
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String(" HEALTH MONITOR ");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String("   USING IOT    ");
    __delay_ms(2000);
    Lcd_Clear();
    while(1){
       temp = adc_read(0);
        press = adc_read(1);
        temp = temp * 0.4887;
        if(temp >= 40){
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String("TEMP HIGH");
        }
        else{
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String("T:");
            Lcd_Set_Cursor(1,3);
            sprintf(t,"%u ",temp);
            Lcd_Write_String(t);
            Lcd_Set_Cursor(1,5);
            Lcd_Write_String("     ");
        }
        if(pbeat != cbeat){
            Lcd_Set_Cursor(1,10);
            Lcd_Write_String("H:");
            Lcd_Set_Cursor(1,13);
            sprintf(h,"%d ",cbeat);
            Lcd_Write_String(h);
            Lcd_Set_Cursor(1,5);
            Lcd_Write_String("   ");
            pbeat = cbeat;
        }
        else{
            Lcd_Set_Cursor(1,10);
            Lcd_Write_String("H:");
            Lcd_Set_Cursor(1,13);
            sprintf(h,"%d ",pbeat);
            Lcd_Write_String(h);
        }
        if(press >200){
            c_press_count++;
            Lcd_Set_Cursor(2,5);
            Lcd_Write_String("R:");
            Lcd_Set_Cursor(2,8);
            sprintf(p,"%d",c_press_count);
            Lcd_Write_String(p);
            p_press_count = c_press_count;
        }
        else{
            Lcd_Set_Cursor(2,5);
            Lcd_Write_String("R:");
            Lcd_Set_Cursor(2,8);
            sprintf(p,"%d",p_press_count);
            Lcd_Write_String(p);
        }
/*-------------------------------------------------------------------------------------*/
        if(count == 90){
            uart_txc(0x2a);
            __delay_ms(2);
            if(temp >= 40){
                uart_txs("HIGH TEMP ");
            }
            else{
                uart_txs("T:");
                uart_txs(t);	
            }			
            if(cbeat >= 150){
                uart_txs("HIGH BEAT ");
            }
            else{
                uart_txs("H:");
                uart_txs(h);	
            }//uart_txc(0x5f);						//IOT Data Upload
            if(c_press_count >= 25){
                uart_txs("HIGH PRESS ");
            }
            else{
                uart_txs("R:");
                uart_txs(p);
            }
            uart_txc(0x23);
            __delay_ms(1000);
        }
        
/*-------------------------------------------------------------------------------------*/
    }
}
static void interrupt T1()
{
    if(TMR1IF)
    {
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