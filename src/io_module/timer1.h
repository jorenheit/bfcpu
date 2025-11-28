#pragma once

#include "settings.h"

namespace Timer1 {

  using ISRPtr = void(*)();

  static ISRPtr _isrPtr = nullptr;
  static constexpr int _supportedPrescalers[5] = {1024, 256, 64, 8, 1};

  // Recursive template to find prescaler and compare match
  template <uint32_t Frequency, uint32_t PrescalerIndex = 0>
  struct FindParams {
    static constexpr uint32_t compare_match_test =
      MCU_FREQUENCY / (_supportedPrescalers[PrescalerIndex] * Frequency) - 1;

    static constexpr bool valid = compare_match_test > 0 && compare_match_test < (1UL << 16);

    static constexpr uint32_t compare_match = 
      valid ? compare_match_test
            : FindParams<Frequency, PrescalerIndex + 1>::compare_match;

    static constexpr uint32_t prescaler =
      valid ? _supportedPrescalers[PrescalerIndex]
            : FindParams<Frequency, PrescalerIndex + 1>::prescaler;
  };

  template <uint32_t Frequency>
  struct FindParams<Frequency, 5> {
    static constexpr uint32_t compare_match = -1;
    static constexpr uint32_t prescaler = -1;
  };

  template <uint32_t Frequency>
  struct Params {
    static constexpr uint32_t compare_match = FindParams<Frequency>::compare_match;
    static constexpr uint32_t prescaler     = FindParams<Frequency>::prescaler;

    static_assert(compare_match != -1, "Could not find suitable parameters");
    static_assert(prescaler     != -1, "Could not find suitable parameters");
  };

  static void stop() {
    TIMSK1 &= ~(1 << OCIE1A); // disable compare match interrupt
  }

  static void restart() {
    TCNT1  = 0;               // reset counter
    TIMSK1 |= (1 << OCIE1A);  // start timer
  }
  
  // Start timer at specified frequency
  template <uint32_t Frequency>
  static void start(ISRPtr isrPtr) {
    if (!isrPtr) return;
    _isrPtr = isrPtr;
    stop();

    TCCR1A = 0; // clear control registers
    TCCR1B = 0;
    TCNT1  = 0; // reset counter
    TCCR1B |= (1 << WGM12);  // CTC mode

    OCR1A = Params<Frequency>::compare_match;
    switch (Params<Frequency>::prescaler) {
      case 1:    TCCR1B |= (1 << CS10); break;
      case 8:    TCCR1B |= (1 << CS11); break;
      case 64:   TCCR1B |= (1 << CS11) | (1 << CS10); break;
      case 256:  TCCR1B |= (1 << CS12); break;
      case 1024: TCCR1B |= (1 << CS12) | (1 << CS10); break;
    }

    restart();
    sei();
  }

  template <uint32_t Frequency, ISRPtr Ptr>
  static void start() {
    static_assert(Ptr != nullptr, "Invalid ISR");
    start<Frequency>(Ptr);
  } 



  // Detach ISR and stop timer
  __attribute__((unused)) static void detach() {
    stop();
    _isrPtr = nullptr;
  }
};

// ISR with nested interrupt support
ISR(TIMER1_COMPA_vect) {
  if (Timer1::_isrPtr) (*Timer1::_isrPtr)();
}