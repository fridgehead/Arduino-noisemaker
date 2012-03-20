#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#define t_RATE 8000

#include "sound.h"

int ledPin = 13;
int speakerPin = 11;
volatile uint16_t t;
volatile unsigned char p = 0;
int param = 4;

// Call the current soundmaking function at 8khz freq
unsigned char (*soundFunc)(int t) = NULL;

ISR(TIMER1_COMPA_vect) {

  p = soundFunc(t);
  OCR2A = p;


  ++t;
}

unsigned char func2(int t){
  return( t >> 4 | t << param & t) ;
}

unsigned char func3(int t){
  return( t << 4 | t >>1 | t) ;
}

unsigned char func4(int t){
  return( t >> 2 | t << 1 & !t) ;
}

unsigned char func(int t){
  return  ((t>>6 & t << 1) | t>> param);
  /// return( t >> 4 | t << param & t) ;
}

void startPlayback()
{
  pinMode(speakerPin, OUTPUT);

  // Set up Timer 2 to do pulse width modulation on the speaker
  // pin.

  // Use internal clock (datasheet p.160)
  ASSR &= ~(_BV(EXCLK) | _BV(AS2));

  // Set fast PWM mode  (p.157)
  TCCR2A |= _BV(WGM21) | _BV(WGM20);
  TCCR2B &= ~_BV(WGM22);

  // Do non-inverting PWM on pin OC2A (p.155)
  // On the Arduino this is pin 11.
  TCCR2A = (TCCR2A | _BV(COM2A1)) & ~_BV(COM2A0);
  TCCR2A &= ~(_BV(COM2B1) | _BV(COM2B0));

  // No prescaler (p.158)
  TCCR2B = (TCCR2B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

  // Set initial pulse width to the first t.
  OCR2A = 2;


  // Set up Timer 1 to send a t every interrupt.

  cli();

  // Set CTC mode (Clear Timer on Compare Match) (p.133)
  // Have to set OCR1A *after*, otherwise it gets reset to 0!
  TCCR1B = (TCCR1B & ~_BV(WGM13)) | _BV(WGM12);
  TCCR1A = TCCR1A & ~(_BV(WGM11) | _BV(WGM10));

  // No prescaler (p.134)
  TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

  // Set the compare register (OCR1A).
  // OCR1A is a 16-bit register, so we have to do this with
  // interrupts disabled to be safe.
  OCR1A = F_CPU / t_RATE;    // 16e6 / 8000 = 2000

  // Enable interrupt when TCNT1 == OCR1A (p.136)
  TIMSK1 |= _BV(OCIE1A);

  t = 0;
  sei();
}

void stopPlayback()
{
  // Disable playback per-t interrupt.
  TIMSK1 &= ~_BV(OCIE1A);

  // Disable the per-t timer completely.
  TCCR1B &= ~_BV(CS10);

  // Disable the PWM timer.
  TCCR2B &= ~_BV(CS10);

  digitalWrite(speakerPin, LOW);
}

void setup()
{
  pinMode(ledPin, OUTPUT);
  digitalWrite(13,HIGH);
  startPlayback();
  Serial.begin(9600);
  soundFunc = &func;

}
char u = 1;
void loop()
{
  char val = (char)map(analogRead(0),480, 1024, 0, 4);

  if (val == 0){

    soundFunc = &func;
  } 
  else if (val == 1){
    soundFunc = &func2;
  } 
  else if (val == 2){
    soundFunc = &func3;

  } 
  else if (val == 3){
    soundFunc = &func4;

  } 
  else {

    soundFunc = &func;
  }





}


