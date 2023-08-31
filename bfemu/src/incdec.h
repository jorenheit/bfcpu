#ifndef INCDEC_H
#define INCDEC_H
#include "module.h"

template <size_t N>
class IncDec: public Module
{
  bool d_dec = false;
  
public:
  void setDec(bool const dec)
  {
    d_dec = dec;
  }

private:
  virtual unsigned long data() const override
  {
    unsigned long data = 0;
    for (int i = 0; i != N; ++i) 
      data |= ((d_input[i] == HIGH ? 1 : 0) << i);
    
    return d_dec ? data - 1 : data + 1;
  }
};


#endif 
