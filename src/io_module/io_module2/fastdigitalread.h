namespace FastDigitalReadImpl_ {
  
  enum Port: uint8_t {
    Invalid = 0,
    PortB, 
    PortC,
    PortD,
    Analog
  };

  constexpr byte port(int Pin) {
    return (Pin >= 8  && Pin <= 13) ? PortB :
           (Pin >= A0 && Pin <= A5) ? PortC :
           (Pin >= 0  && Pin <= 7)  ? PortD :
           (Pin == A6 || Pin == A7) ? Analog : Invalid;
  }

  template <int Pin, int Threshold, int Port>
  struct Read;

  template <int Pin, int Threshold>
  struct Read<Pin, Threshold, PortB> {
    static inline __attribute__((always_inline)) bool read() {
      return (PINB >> (Pin - 8)) & 1;
    }
  };

  template <int Pin, int Threshold>
  struct Read<Pin, Threshold, PortC> {
    static inline __attribute__((always_inline)) bool read() {
      return (PINC >> (Pin - A0)) & 1;
    }
  };

  template <int Pin, int Threshold>
  struct Read<Pin, Threshold, PortD> {
    static inline __attribute__((always_inline)) bool read() {
      return (PIND >> (Pin - 0)) & 1;
    }
  };

  template <int Pin, int Threshold>
  struct Read<Pin, Threshold, Analog> {
    static inline __attribute__((always_inline)) bool read() {
      return analogRead(Pin) > Threshold;
    }
  };
}

template <int Pin, int Threshold = 512>
inline __attribute__((always_inline))  bool fastDigitalRead() {
  static constexpr byte PORT = FastDigitalReadImpl_::port(Pin); 
  static_assert(PORT != FastDigitalReadImpl_::Invalid, "Invalid pin (0 - A7)");
  return FastDigitalReadImpl_::Read<Pin, Threshold, PORT>::read();
}

