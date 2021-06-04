//ME433 - HW8
#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include "ic2.h"
#include <math.h>

// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

//Define addresses manually unlike PIC32, MCP23017 needs to be defined 
#define IODIRA 0x00 
#define IODIRB 0xFF 
#define read_add 0b01000001
#define write_add 0b01000000
#define OLATA 0x14
#define GPIOB 0x13
void setPin(unsigned char address, unsigned char _register, unsigned char value){
    //start bit 
    //address
    i2c_master_start(); 
    i2c_master_send(address); 
    i2c_master_send(_register); // need a byte in this function -- reg then value
    i2c_master_send(value); //value
    i2c_master_stop(); 
}

unsigned char readPin(unsigned char w_address, unsigned char r_address, unsigned char _register){
    i2c_master_start(); 
    i2c_master_send(w_address);
    i2c_master_send(_register); 
    i2c_master_restart(); 
    i2c_master_send(r_address); 
    unsigned char c = i2c_master_recv(); 
    i2c_master_ack(1); 
    i2c_master_stop(); 
    
    return c;
}
int main(){
    
     __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISAbits.TRISA4 = 0; //A4 as output
    TRISBbits.TRISB4 = 1; //B4 as input 
    
    LATAbits.LATA4 = 0; 
    LATBbits.LATB4 = 0; //both A4 and B4 turned off 
    
    __builtin_enable_interrupts();
    i2c_master_setup();
    setPin(write_add, IODIRA, 0x00); 
    setPin(write_add, IODIRB, 0xFF); 
    while(1){
        if(readPin(write_add, read_add, GPIOB)&0b1  == 0b1){
            setPin(write_add, OLATA, 0x00);
        }
        else{
            setPin(write_add, OLATA, 0xFF); 
        }
        
        LATAbits.LATA4 = 0;
        //setPin(write_add, OLATA, 0x00);
        _CP0_SET_COUNT(0);
            
        while (_CP0_GET_COUNT() < 48000000/8){;}
      
        LATAbits.LATA4 = 1;
        //setPin(write_add, OLATA, 0xFF);
        _CP0_SET_COUNT(0);
    }
}