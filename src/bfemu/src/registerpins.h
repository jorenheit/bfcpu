#ifndef REGISTERPINS_H
#define REGISTERPINS_H


template <size_t N>
struct RegisterPins;

template <>
struct RegisterPins<4>
{
  using ValueType = uint8_t;
    
  enum Input {
    D0, D1, D2, D3, 
    EN, LD, CNT, DEC,
    N_INPUT,
    DATA_IN = mask(D0, D1, D2, D3),
  };

  enum Output {
    Q0, Q1, Q2, Q3, 
    CA,
    Z, 
    N_OUTPUT,
    DATA_OUT = mask(Q0, Q1, Q2, Q3),
  };
};

template <>
struct RegisterPins<8>
{
  using ValueType = uint8_t;
    
  enum Input {
    D0, D1, D2, D3, D4, D5, D6, D7, 
    EN, LD, CNT, DEC,
    N_INPUT,
    DATA_IN = mask(D0, D1, D2, D3, D4, D5, D6, D7),
  };

  enum Output {
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, 
    CA,
    Z, 
    N_OUTPUT,
    DATA_OUT = mask(Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7),
  };
};

template <>
struct RegisterPins<16>
{
  using ValueType = uint16_t;
    
  enum Input {
    D0, D1, D2, D3, D4, D5, D6, D7, 
    D8, D9, D10, D11, D12, D13, D14, D15,
    
    EN, LD, CNT, DEC,
    N_INPUT,
    DATA_IN_LOW = mask(D0, D1, D2, D3, D4, D5, D6, D7),
    DATA_IN_HIGH = mask(D8, D9, D10, D11, D12, D13, D14, D15),
    DATA_IN = DATA_IN_HIGH | DATA_IN_LOW
  };

  enum Output {
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, 
    Q8, Q9, Q10, Q11, Q12, Q13, Q14, Q15,
    CA,
    Z, 

    N_OUTPUT,
    DATA_OUT_LOW = mask(Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7),
    DATA_OUT_HIGH = mask(Q8, Q9, Q10, Q11, Q12, Q13, Q14, Q15),
    DATA_OUT = DATA_OUT_HIGH | DATA_OUT_LOW
  };
};  


#endif //REGISTERPINS_H
