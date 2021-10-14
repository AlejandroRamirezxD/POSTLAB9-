/*
  Archivo:  POSTLAB9.c
  Autor:    Alejandro Ramirez Morales
  Creado:   27/sep/21
  Leyendo dos pots en RE0 y RE1 y mostrando en PORTA Y PORTD    
 */

// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

#include <xc.h>
#include <stdint.h>
#define _XTAL_FREQ 8000000

/*
 +----------------------------------------------------------------------------+
 |                                VARIABLES                                   |
 +----------------------------------------------------------------------------+
 */
uint8_t val_adresh;
uint8_t val_adreshp;
int msegundos = 0;
/*
 +----------------------------------------------------------------------------+
 |                          PROTOTIPOS DE FUNCIONES                           |
 +----------------------------------------------------------------------------+
 */
void setup(void);

/*
 +----------------------------------------------------------------------------+
 |                               INTERRUPCIONES                               |
 +----------------------------------------------------------------------------+
 */
void __interrupt() isr (void)
{
     if(PIR1bits.ADIF)         // ADC
    {    // interrupción para cambiar el valor del pwm 
         
         if(ADCON0bits.CHS == 6)
         {
             CCPR1L = (ADRESH>>1)+124; // Bits mas significativos
             CCP1CONbits.DC1B1 = ADRESH & 0b01; // Bits menos significativos
             CCP1CONbits.DC1B0 = (ADRESH >> 7);
         }
         else if (ADCON0bits.CHS == 5)
         {
             CCPR2L = (ADRESH>>1)+124; // Bits mas significativos
             CCP2CONbits.DC2B0 = ADRESH & 0b01; // Bits menos significativos
             CCP2CONbits.DC2B0 = (ADRESH >> 7);
         }
         else if (ADCON0bits.CHS == 7)
         {
             val_adresh = ADRESH;
         }
         
         
         
         
         PIR1bits.ADIF = 0;     // Apagar bandera
    }  
    else if(INTCONbits.T0IF) 
    {
         INTCONbits.T0IF = 0;
         TMR0 = 56; 
         
         msegundos = msegundos+1;
         
         if(msegundos == 26)
         {
             msegundos = 0;  
         }
         
         if(msegundos < val_adreshp)
         {
             PORTD = 0b01;
         }
         else
         {
             PORTD = 0b0;
         }    
             
}
}

/*
 +----------------------------------------------------------------------------+
 |                                   LOOP                                     |
 +----------------------------------------------------------------------------+
 */
void main(void) 
{
    setup(); // Se ejecuta funcion setup
    ADCON0bits.GO = 1; // El ciclo A/D esta en progreso
    while(1)
    {     
        if (ADCON0bits.GO == 0){
            if(ADCON0bits.CHS == 7)
                ADCON0bits.CHS = 6;
            
            else if (ADCON0bits.CHS == 6)
                ADCON0bits.CHS = 5;
            
            else if (ADCON0bits.CHS == 5)
                ADCON0bits.CHS = 7;
            
            __delay_us(50);
            ADCON0bits.GO = 1;
        }
        val_adreshp = (int)val_adresh/10;
    }
}

/*
 +----------------------------------------------------------------------------+
 |                                  SETUP                                     |
 +----------------------------------------------------------------------------+
 */
void setup(void)
{
    // Ports 
    ANSEL   =   0b11100000;              // Digital Ports
    ANSELH  =   0;
    
    TRISE   =   0b111;          // PORTE - entrada
    TRISD   =   0;
    PORTE   =   0;              // PORTE en 0
    PORTD   =   0;              // PORTE en 0
    
    // Reloj
    OSCCONbits.IRCF = 0b0111;    // 8MHz
    OSCCONbits.SCS = 1;         // Activar reloj interno
    
    // Configuración del ADC
    ADCON1bits.ADFM  = 0;   // Justificado a la izquierda
    ADCON1bits.VCFG1 = 0;   // Referencia como tierra
    ADCON1bits.VCFG0 = 0;   // Referencia poder
    
    ADCON0bits.ADCS = 0b01; // Fosc/32
    ADCON0bits.CHS  = 5;    // Ansel 5
    ADCON0bits.ADON = 1;    // ADC activo
    __delay_us(50);
    
    // Configuración del pwm
    TRISCbits.TRISC2 = 1;   // CCP1 como entrada
    TRISCbits.TRISC1 = 1;   // CCP2 como entrada
    PR2 = 249;              // Para un ciclo de 2ms
   
    // Configuracion de CCP1
    CCP1CONbits.P1M = 0;    // Salida simple
    CCP1CONbits.CCP1M = 0b1100; // PMW activados en high
    CCPR1L = 0x0f; // Ciclo de trabajo inicial
    CCP1CONbits.DC1B = 0;
    
    // Configuracion de CCP2
    CCPR2L = 0x0f;
    CCP2CONbits.CCP2M = 0b1100; // PMW 
    CCP2CONbits.DC2B0 = 0; // PWM DUTY CYCLE
    CCP2CONbits.DC2B1 = 0; // 
    
    // Configuracion TMR2
    PIR1bits.TMR2IF = 0;        // Bandera apagada
    T2CONbits.T2CKPS = 0b11;    // Prescaler 1:16
    T2CONbits.TMR2ON = 1;       // Activar TMR2
    while   (PIR1bits.TMR2IF == 0);
    PIR1bits.TMR2IF == 0;
    
    // Pines de pwm
    TRISCbits.TRISC2 = 0; // Salida de pwm
    TRISCbits.TRISC1 = 0; // Salida de pwm
    
    // TMR0
   //Timer0 Registers Prescaler= 1 - TMR0 Preset = 56 - Freq = 10000.00 Hz - Period = 0.000100 seconds
    OPTION_REGbits.T0CS = 0;  // bit 5  TMR0 Clock Source Select bit...0 = Internal Clock (CLKO) 1 = Transition on T0CKI pin
    OPTION_REGbits.T0SE = 0;  // bit 4 TMR0 Source Edge Select bit 0 = low/high 1 = high/low
    OPTION_REGbits.PSA = 1;   // bit 3  Prescaler Assignment bit...0 = Prescaler is assigned to the WDT
    OPTION_REGbits.PS2 = 0;   // bits 2-0  PS2:PS0: Prescaler Rate Select bits
    OPTION_REGbits.PS1 = 0;
    OPTION_REGbits.PS0 = 0;
    TMR0 = 56;  
    
    // Configuración de las interrupciones
    PIR1bits.ADIF = 0;      // A/D conversion no ha empezado o completado
    PIE1bits.ADIE = 1;      // Activa la interrupción de ADC
    INTCONbits.PEIE = 1;    // 
    INTCONbits.GIE = 1;
    INTCONbits.T0IE = 1;        // Activar interrupcion de Tmr0
    INTCONbits.TMR0IF = 0;
    return;   
}


 