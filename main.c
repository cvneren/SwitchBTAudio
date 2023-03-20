// PIC16F1503 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection Bits (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LPBOR = OFF      // Low-Power Brown Out Reset (Low-Power BOR is disabled)
#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 4000000
#define LED_in RC1
#define LED_out RA4
#define Button_out RC2
#define VOL_Up RC4
#define VOL_Down RC3
#define HP_Sense RC5

uint8_t TMR0_LED_enable = 0;
uint8_t TMR0_button_enable = 0;
uint16_t TMR0_count = 0;
uint8_t connected;

void PIC_Setup() {
    
    // Internal oscillator enabled at 4 MHz
    OSCCON = 0b11101010;
    
    // Pull-ups disabled, Timer0 has a prescaler of 16 with clock source on RA2
    OPTION_REG = 0b11110011;
    TMR0IF = 0;
    
    // A4, C2, and C5 are output, C1, C3, and C4 are input (C5 and C2 are temporarily inputs)
    TRISA = 0x00;
    TRISC = 0b00111110;
    
    // All IOs are digital
    ANSELA = 0x00;
    ANSELC = 0x00;
    
    LED_out = 0;
}

void Clear_Button_Timer() {
    
    TMR0CS = 1;
    TMR0 = 0x00;
    TMR0_count = 0;
    TMR0_button_enable = 0;
}

void Clear_LED_Timer() {
    
    TMR0CS = 1;
    TMR0 = 0x00;
    TMR0_count = 0;
    TMR0_LED_enable = 0;
}

void Check_Button() {
	
    // Begin timer if it is not running already and both VOL_Up/VOL_Down buttons are pressed
    if ((VOL_Up == 0) && (VOL_Down == 0)) {
        
        if (TMR0_button_enable == 0) {
            TMR0_count = 0;
        
            // Set Timer0 clock to internal instruction cycle to begin counting
            TMR0CS = 0;
            TMR0_button_enable = 1;
        
            // Clear Timer0
            TMR0 = 0x00;
            TMR0IF = 0;
        }
    }
    
    if (TMR0IF && TMR0_button_enable) {
        
        TMR0_count++;
            
        if (TMR0_count == 100) {
            if (VOL_Up || VOL_Down)
                Clear_Button_Timer();
        }
        
        if (TMR0_count == 200) {
            if (VOL_Up || VOL_Down)
                Clear_Button_Timer();
        }
        
        if (TMR0_count >= 300) {
            if ((VOL_Up == 0) && (VOL_Down == 0)) {
                TRISC2 = 0;
                Button_out = 0;
                __delay_ms(1000);
                TRISC2 = 1;
            }
            else
                TRISC2 = 1;
            
            Clear_Button_Timer();
        }
        
        TMR0IF = 0;
    }    
}

void Check_LED() {
    
    if(connected && (LED_in == 0)) {
        connected = 0;
        LED_out = 0;
        TRISC5 = 1;
    }
    
    // Begin timer if it is not running already running, the LED is on, and the previous connection state is 0
    if ((LED_in && (TMR0_LED_enable == 0)) && !connected) {

        TMR0_count = 0;
        
        // Set Timer0 clock to internal instruction cycle to begin counting
        TMR0CS = 0;
        TMR0_LED_enable = 1;
        
        // Clear Timer0
        TMR0 = 0x00;
        TMR0IF = 0;
    }
    
    // Allow the timer to keep counting in case it was turned off by the button sequence
    if (TMR0_LED_enable && (TMR0CS == 1))
        TMR0CS = 0;
    
    // Check the LED state at random intervals (measured in milliseconds) so it doesn't pick up on the LED's natural blinking
    if (TMR0IF && TMR0_LED_enable) {
        
        TMR0_count++;
        
        if (TMR0_count == 100) {
            if (LED_in == 0)
                Clear_LED_Timer();
        }
        
        if (TMR0_count == 293) {
            if (LED_in == 0)
                Clear_LED_Timer();
        }
            
        if (TMR0_count == 345) {
            if (LED_in == 0)
                Clear_LED_Timer();
        }
        
        if (TMR0_count == 598) {
            if (LED_in == 0)
                Clear_LED_Timer();
        }
        
        if (TMR0_count >= 612) {
            if (LED_in) {
                connected = 1;
                LED_out = 1;
                TRISC5 = 0;
                HP_Sense = 0;
            }
            else {
                connected = 0;
                LED_out = 0;
                TRISC5 = 1;
            }
            Clear_LED_Timer();
        }
        
        TMR0IF = 0;
    }    
}

void main() {
    
    PIC_Setup();   
  
    while(1) { 
        
        Check_Button();
        
        if(TMR0_button_enable == 0)
            Check_LED();
    }
}
