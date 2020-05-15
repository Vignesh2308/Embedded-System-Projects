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