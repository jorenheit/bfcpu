#pragma once

extern volatile size_t tickCount;

inline void tic() {
  tickCount = 0;
}

inline size_t toc() {
  return tickCount;
}

double measureFrequency() {
  static double lastKnownFrequency = 0;
  static unsigned long lastMeasured = 0;

  unsigned long currentTime = millis();
  if (lastMeasured && currentTime - lastMeasured < FREQUENCY_UPDATE_INTERVAL) {
    return lastKnownFrequency;
  }

  tic();
  while (millis() - currentTime < FREQUENCY_MEASUREMENT_TIME) {}
  uint32_t const count = toc();

  lastMeasured = currentTime + FREQUENCY_MEASUREMENT_TIME;
  return (lastKnownFrequency = static_cast<double>(count) / FREQUENCY_MEASUREMENT_TIME);
}

template <unsigned long(TimeFunc)() = millis>
void interruptable_delay(size_t const t) {
  unsigned long now = TimeFunc();
  while (TimeFunc() - now < t) {}
}

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
void shiftOut(uint8_t const value)
{
  for (uint8_t i = 0; i < 8; ++i)  {
    digitalWrite<DataPin>(BitOrder == LSBFIRST ? 
      value & (1 << i) :
      value & (1 << (7 - i))
    );
        
    digitalWrite<ClockPin, HIGH>();
    digitalWrite<ClockPin, LOW>();        
  }
}