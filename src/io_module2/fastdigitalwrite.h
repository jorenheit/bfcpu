namespace FastDigitalWriteImpl_ {

  enum Port: byte {
    Invalid = 0,
    PortB, 
    PortC,
    PortD
  };

  constexpr byte port(int Pin) {
    return (Pin >= 8  && Pin <= 13) ? PortB :
           (Pin >= A0 && Pin <= A5) ? PortC :
           (Pin >= 0  && Pin <= 7)  ? PortD : Invalid;
  }


  template <int Pin, int Port, bool State>
  struct Write;

  // State known at compiletime
  template <int Pin>
  struct Write<Pin, PortB, LOW> {
    static inline __attribute__((always_inline)) void write() {
      PORTB &= ~(1 << (Pin - 8));
    }
  };

  template <int Pin>
  struct Write<Pin, PortB, HIGH> {
    static inline __attribute__((always_inline)) void write() {
      PORTB |= (1 << (Pin - 8));
    }
  };

  template <int Pin>
  struct Write<Pin, PortC, LOW> {
    static inline __attribute__((always_inline)) void write() {
      PORTC &= ~(1 << (Pin - A0));
    }
  };

  template <int Pin>
  struct Write<Pin, PortC, HIGH> {
    static inline __attribute__((always_inline)) void write() {
      PORTC |= (1 << (Pin - A0));
    }
  };

  template <int Pin>
  struct Write<Pin, PortD, LOW> {
    static inline __attribute__((always_inline)) void write() {
      PORTD &= ~(1 << (Pin - 0));
    }
  };

  template <int Pin>
  struct Write<Pin, PortD, HIGH> {
    static inline __attribute__((always_inline)) void write() {
      PORTD |= (1 << (Pin - 0));
    }
  };
}

template <int Pin, bool State>
inline __attribute__((always_inline)) void fastDigitalWrite() {
  static constexpr byte PORT = FastDigitalWriteImpl_::port(Pin); 
  static_assert(PORT != FastDigitalWriteImpl_::Invalid, "Invalid pin (0 - A5)");
  FastDigitalWriteImpl_::Write<Pin, PORT, State>::write();
}

template <int Pin>
inline __attribute__((always_inline)) void fastDigitalWrite(bool const state) {
  if (state) fastDigitalWrite<Pin, HIGH>();
  else       fastDigitalWrite<Pin, LOW>();
}
