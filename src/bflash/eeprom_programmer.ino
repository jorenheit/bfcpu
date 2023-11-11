

enum Pins {
  SHIFT_DATA = 2,
  SHIFT_LATCH = 3,
  SHIFT_CLOCK = 4,
  
  IO_0 = 5,
  IO_7 = 12,

  CE = 13
};

enum ReadWrite {
  READ,
  WRITE
};

int generateShiftRegisterContents(int addr, ReadWrite RW) {

  // Bit: 15 14 13  12  11 10 09 08 07 06 05 04 03 02 01 00
  //      OE WE -- A12 A11 ...                       ... A0

  // Clear bits 13, 14, 15 (AND with 0x1fff) and set either bit 15 or 14 depending 
  // on if we're reading or writing:
  
  return (addr & 0x1fff) | (RW == READ ? (1 << 15) : (1 << 14));
}

void setShiftRegisters(int const addr, ReadWrite const RW) {
  //      |   Shift Register 1 (msb)  |  Shift Register 0 (lsb) |
  // Bit: | 15 14 13  12  11 10 09 08 | 07 06 05 04 03 02 01 00 |
  //      | OE WE -- A12 A11  ...  A8 | A7 ...           ... A0 |

  // Clear bits 13, 14, 15 (AND with 0x1fff) and set either bit 15 or 14 depending 
  // on if we're reading or writing:
  int const value16 = (addr & 0x1fff) | (RW == READ ? (1 << 15) : (1 << 14));

  // Shift the least significant byte in first, so that one will end up on the right
  // after shifting in the most significant byte, leaving us in a little endian representation.
  shiftOut(SHIFT_DATA, SHIFT_CLOCK, MSBFIRST, (value16 >> 8) & 0xff); 
  shiftOut(SHIFT_DATA, SHIFT_CLOCK, MSBFIRST, (value16 >> 0) & 0xff);

  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}

void setIOPinsAs(int mode) {
  for (int pin = IO_0; pin <= IO_7; ++pin) {
    pinMode(pin, mode);
  }
}

byte readEEPROM(int addr, bool disableAfterRead = true) {
  int values = generateShiftRegisterContents(addr, READ);
  byte msb = (values >> 8) & 0xff;
  byte lsb = (values >> 0) & 0xff;

  // Make {address, OE = 1, WE = 0} available on pins of the EEPROM
  setShiftRegisters(msb, lsb);
  
  // Enable EEPROM -> contents will become available on the input;
  digitalWrite(CE, LOW);

  // Read data into return variable 
  byte data = 0;
  setIOPinsAs(INPUT);
  for (int pin = IO_0; pin <= IO_7; ++pin) {
    data |= (digitalRead(pin) << (pin - IO_0));
  }

  if (disableAfterRead) {
    digitalWrite(CE, HIGH);
  }

  return data;
}

void readEEPROM(int start, int nBytes, byte *buf) {
  for (int i = 0; i != nBytes; ++i) {
    buf[i] = readEEPROM(start + i, /*disableAfterRead*/ false);
  }

  digitalWrite(CE, HIGH); // disable EEPROM
}

void writeEEPROM(int addr, byte value) {

  // Make sure EEPROM is disabled while setting the outputs of the arduino
  digitalWrite(CE, HIGH);
  setIOPinsAs(OUTPUT);
  for (int pin = IO_0; pin <= IO_7; ++pin) {
    digitalWrite(pin, (value >> (pin - IO_0)) & 1);
  }

  // Make {address, OE = 0, WE = 1} available on pins of the EEPROM
  auto [msb, lsb] = generateShiftRegisterContents(addr, WRITE);
  setShiftRegisters(msb, lsb);

  // Pulse CE of the EEPROM to write the value into it
  digitalWrite(CE, LOW);
  delayMicroseconds(1); // CE needs to be held at least 100ns
  digitalWrite(CE, HIGH);

  // set IO back to input to prevent accidental writes
  setIOPinsAs(INPUT);
}

void setup() {
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  pinMode(SHIFT_CLOCK, OUTPUT);
  setIOPinsAs(INPUT);
  pinMode(CE, OUTPUT);
  digitalWrite(CE, HIGH); // disable chip 
}

void loop() {
  

}
