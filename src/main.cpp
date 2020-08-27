/*******************************************************
This program was created by the
CodeWizardAVR V3.12 Advanced
Automatic Program Generator
© Copyright 1998-2014 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : 
Version : 
Date    : 26.04.2019
Author  : 
Company : 
Comments: 


Chip type               : ATtiny13A
AVR Core Clock frequency: 9,600000 MHz
Memory model            : Tiny
External RAM size       : 0
Data Stack size         : 16
*******************************************************/

// FUSES high - 0xFF low - 0x7A

// #include <tiny13a.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>

// Declare your global variables here
#define KEY           (PINB & _BV(PB0))
#define PWM           PB1
#define RESET_BUTTON  PB4

#define _SET(bit)    PORTB |= (1 << bit)
#define _CLEAR(bit)    PORTB &= ~(1 << bit)

// длинное нажатие * 0,04 мс 
#define LONG_KEYPRESS 50000
// короткое нажатие * 0,04 мс
#define KEYPRESS 500
// длительность RESET * 0,04 мс
#define RESET_DELAY 6250
// длительность двойного нажатия * 0,04 мс
#define DOUBLECLICK_DELAY 10000
// задержка изменения PWM
#define PWM_DELAY_STARTUP 1250
#define PWM_DELAY_UP 2500
#define PWM_DELAY_DOWN 1250

#define SET_PWM(x)                                                                           \
OCR0B = x;                                                                                   \
if (x == 0) {                                                                                \
    TCCR0A=(0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (1<<WGM01) | (1<<WGM00);  \
    _CLEAR(PWM);                                                                             \
} else {                                                                                     \
    TCCR0A=(0<<COM0A1) | (0<<COM0A0) | (1<<COM0B1) | (0<<COM0B0) | (1<<WGM01) | (1<<WGM00);  \
}                                                                                            \

unsigned int key_pressed_counter, doubleclick_counter, pwm_delay_counter;

#define MAX_PWM_VALUES 4
static unsigned char pwm_values[MAX_PWM_VALUES] = {0, 0x0C, 0x1E, 0x2F};

EEMEM unsigned char e_pwm_index = 1;
unsigned char pwm_index;

unsigned char key_pressed, singleclick;

unsigned char pwm;

unsigned char startup;

// Timer 0 overflow interrupt service routine
ISR(TIM0_OVF_vect)
{
  if (pwm_delay_counter > 0) {
    pwm_delay_counter--;
  } else {
    unsigned char _pwm, _ocr0b;
    _pwm = startup ? OCR0A : pwm;
    _ocr0b = OCR0B;
    if (OCR0B < _pwm) {
      pwm_delay_counter = startup ? PWM_DELAY_STARTUP : PWM_DELAY_UP;
      _ocr0b = OCR0B + 1;
    } else if (OCR0B > _pwm) {
      pwm_delay_counter = PWM_DELAY_DOWN;
      _ocr0b = OCR0B - 1;
    } else if (startup) {
      startup = 0;
    }
    SET_PWM(_ocr0b);
  }

  if (key_pressed && key_pressed_counter < 65535) {
    key_pressed_counter++;
  }
                 
  if (doubleclick_counter > 0) {
    doubleclick_counter--;
    singleclick = doubleclick_counter == 0;
  }

// Place your code here
  if (KEY == 0) {
    key_pressed = 1;
    if (key_pressed_counter > LONG_KEYPRESS) {
      // длинное нажатие продолжается
      if ((key_pressed_counter - LONG_KEYPRESS) < RESET_DELAY) {
        _CLEAR(RESET_BUTTON);
      } else {
        _SET(RESET_BUTTON);
      }
    }
  } else {
    key_pressed = 0;
    if (key_pressed_counter > LONG_KEYPRESS) {
      // длинное нажатие закончено
      singleclick = 0;
      _SET(RESET_BUTTON);
    } else if (key_pressed_counter > KEYPRESS) {
      if (doubleclick_counter > 0) {
      // двойное нажатие с защитой от дребезга
        doubleclick_counter = 0;        
        // запись в eeprom
        eeprom_write_byte(&e_pwm_index, pwm_index);
      } else {
        doubleclick_counter = DOUBLECLICK_DELAY;
      }
    } else if (singleclick) {
      // короткое нажатие с защитой от дребезга
      singleclick = 0;
      pwm_index++; 
      if (pwm_index > (MAX_PWM_VALUES - 1)) {
        pwm_index = 0;
      }
      startup = pwm == 0 && OCR0B < pwm_values[pwm_index];
      pwm = pwm_values[pwm_index];
    }
    key_pressed_counter = 0;
  }

}

int main(void)
{
// Declare your local variables here

// Crystal Oscillator division factor: 1
CLKPR=(1<<CLKPCE);
CLKPR=(0<<CLKPCE) | (0<<CLKPS3) | (0<<CLKPS2) | (0<<CLKPS1) | (0<<CLKPS0);

// Input/Output Ports initialization
// Port B initialization
/*
// Function: Bit5=In Bit4=In Bit3=In Bit2=Out Bit1=Out Bit0=In 
DDRB=(0<<DDB5) | (0<<DDB4) | (0<<DDB3) | (1<<DDB2) | (1<<DDB1) | (0<<DDB0);
// State: Bit5=T Bit4=T Bit3=T Bit2=1 Bit1=0 Bit0=P 
PORTB=(0<<PORTB5) | (0<<PORTB4) | (0<<PORTB3) | (1<<PORTB2) | (0<<PORTB1) | (1<<PORTB0);
*/
// Function: Bit5=In Bit4=Out Bit3=In Bit2=In Bit1=Out Bit0=In 
DDRB=(0<<DDB5) | (1<<DDB4) | (0<<DDB3) | (0<<DDB2) | (1<<DDB1) | (0<<DDB0);
// State: Bit5=T Bit4=1 Bit3=T Bit2=T Bit1=0 Bit0=P 
PORTB=(0<<PORTB5) | (1<<PORTB4) | (0<<PORTB3) | (0<<PORTB2) | (0<<PORTB1) | (1<<PORTB0);


// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 1200,000 kHz
// Mode: Fast PWM top=OCR0A
// OC0A output: Disconnected
// OC0B output: Non-Inverted PWM
// Timer Period: 0,04 ms
// Output Pulse(s):
// OC0B Period: 0,04 ms Width: 0 us
TCCR0A=(0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (1<<WGM01) | (1<<WGM00);
TCCR0B=(1<<WGM02) | (0<<CS02) | (1<<CS01) | (0<<CS00);
TCNT0=0x00;
OCR0A=0x2F;
OCR0B=0x00;

// Timer/Counter 0 Interrupt(s) initialization
TIMSK0=(0<<OCIE0B) | (0<<OCIE0A) | (1<<TOIE0);

// External Interrupt(s) initialization
// INT0: Off
// Interrupt on any change on pins PCINT0-5: Off
GIMSK=(0<<INT0) | (0<<PCIE);
MCUCR=(0<<ISC01) | (0<<ISC00);

// Analog Comparator initialization
// Analog Comparator: Off
// The Analog Comparator's positive input is
// connected to the AIN0 pin
// The Analog Comparator's negative input is
// connected to the AIN1 pin
ACSR=(1<<ACD) | (0<<ACBG) | (0<<ACO) | (0<<ACI) | (0<<ACIE) | (0<<ACIS1) | (0<<ACIS0);
ADCSRB=(0<<ACME);
// Digital input buffer on AIN0: On
// Digital input buffer on AIN1: On
DIDR0=(0<<AIN0D) | (0<<AIN1D);

// ADC initialization
// ADC disabled
ADCSRA=(0<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (0<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);

pwm_index = eeprom_read_byte(&e_pwm_index);
pwm_index = (pwm_index >= 0 && pwm_index <= 3) ? pwm_index : 1;
pwm = pwm_values[pwm_index];

startup = pwm != 0;
// fan start
//PWM = 1;
//OCR0B = 0x2F;
//delay_ms(2000);

// Global enable interrupts
sei();

while (1)
      {
      // Place your code here

      }
}
