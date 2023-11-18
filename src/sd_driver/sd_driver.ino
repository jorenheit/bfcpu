#include <SD.h>


/*

Input to the module: 
instruction pointer -> 16 bit input -> need 2x 8 bit multiplexer (74LS151)
Connect the select lines (3x) together and run the outputs of the 151 to 2 different input pins to 
read the 2 bytes in parallel into memory. Something like:

uint16_t getAddress() {
  uint16_t lowByte = 0;
  uint16_t highByte = 0;
  for (int bit = 0; bit != 8; ++bit) {
    digitalWrite(SELECT0, bit & 1);
    digitalWrite(SELECT1, bit & 2);
    digitalWrite(SELECT2, bit & 4);

    lowByte |= (digitalRead(LOW_BYTE_BIT) << bit);
    highByte |= (digitalRead(HIGH_BYTE_BIT) << bit);
  }

  return (highByte << 8) | lowByte;
}

Number of pins needed: 3 for bit-select, 2 inputs -> 5
This leaves enough pins for parallel output of the result without using shift registers.
This allows us to fetch an address, which we use as an index into the file.

uint16_t addr = getAddress();
SD.seek(addr);
byte value = SD.peek();
--> output this value to output pins

Should this happen in an interrupt? I guess so -> 1 additional pin needed
*/


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
