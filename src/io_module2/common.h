#pragma once

inline __attribute__((always_inline)) 
void setIOPinsToOutput() {
  DDRC |= B00000001;
  DDRB |= B00111111;
  DDRD |= B10000000;
}

inline __attribute__((always_inline)) 
void setIOPinsToInput() {
  DDRC &= ~B00000001;
  DDRB &= ~B00111111;
  DDRD &= ~B10000000;
} 

inline __attribute__((always_inline)) 
byte readByteFromBus() {
  // !! Assumes datapins have been set to input !!
  return ((PINC & B00000001) << 7) | 
	       ((PINB & B00111111) << 1) | 
	       ((PIND & B10000000) >> 7);
}

inline __attribute__((always_inline)) 
void writeByteToBus(byte data) {
  // !! Assumes datapins have been set to output !!
  PORTC = (PORTC & ~B00000001) | ((data & B10000000) >> 7);
  PORTB = (PORTB & ~B00111111) | ((data & B01111110) >> 1);
  PORTD = (PORTD & ~B10000000) | ((data & B00000001) << 7);
}

#include "fastdigitalread.h"
template <int Pin>
inline __attribute__((always_inline))  
bool digitalRead() {
  return fastDigitalRead<Pin>();
}

#include "fastdigitalwrite.h"
template <int Pin>
inline __attribute__((always_inline))  
void digitalWrite(bool state) {
  return fastDigitalWrite<Pin>(state);
}

template <int Pin, bool State>
inline __attribute__((always_inline))  
void digitalWrite() {
  return fastDigitalWrite<Pin, State>();
}

template <int DataPin, int ClockPin, int BitOrder>
void shiftOut(uint8_t val)
{
  for (uint8_t i = 0; i < 8; ++i)  {
    digitalWrite<DataPin>(BitOrder == LSBFIRST ? 
      val & (1 << i) :
      val & (1 << (7 - i))
    );
        
    digitalWrite<ClockPin, HIGH>();
    digitalWrite<ClockPin, LOW>();        
  }
}