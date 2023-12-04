#define MAX_ROM_SIZE (1 << 13)

enum Pins {
  SHIFT_DATA = 13,
  SHIFT_LATCH = 11,
  SHIFT_CLOCK = 12,
  
  CE = 2,

  STATUS_LED = A0,
};

int IO_PINS[8] = {
  3, 4, 5, 6, 7, 8, 9, 10
};

enum ReadWrite {
  READ,
  WRITE,
  NONE
};

enum Signals {
  DUMP_REQUEST = 0,
  FLASH_REQUEST = 1,
  PROGRAMMER_READY = 2,
  PROGRAMMER_FINISHED = 3
};

void setShiftRegisters(int const addr, ReadWrite const RW) {
  //      |   Shift Register 1 (msb)  |  Shift Register 0 (lsb) |
  // Bit: | 15 14 13  12  11 10 09 08 | 07 06 05 04 03 02 01 00 |
  //      | OE WE -- A12 A11  ...  A8 | A7 ...           ... A0 |


  // Set bits 13, 14, 15 (OR with 0xE000 = 0b1110...0) and clear either bit 14 or 15 depending 
  // on if we're reading or writing:
  int value16 = (addr | 0xE000);
  if (RW != NONE) {
    value16 &= (RW == WRITE) ? ~(1 << 14) : ~(1 << 15);
  }

  // Shift the least significant byte in first, so that one will end up on the right
  // after shifting in the most significant byte, leaving us in a little endian representation.
  shiftOut(SHIFT_DATA, SHIFT_CLOCK, MSBFIRST, (value16 >> 8) & 0xff); 
  shiftOut(SHIFT_DATA, SHIFT_CLOCK, MSBFIRST, (value16 >> 0) & 0xff);

  // Latch result onto the pins
  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  delayMicroseconds(10);
  digitalWrite(SHIFT_LATCH, LOW);
}

void setOutput(byte const value) {
  setIOPinsAs(OUTPUT);
  for (int i = 0; i != 8; ++i) {
    int const state = (value >> i) & 1 ? HIGH : LOW;
    digitalWrite(IO_PINS[i], state);
  }
}

void setIOPinsAs(int mode) {
  for (int i = 0; i != 8; ++i) {
    pinMode(IO_PINS[i], mode);
  }
}

void enableEEPROM() {
  digitalWrite(CE, LOW);
}

void disableEEPROM() {
  digitalWrite(CE, HIGH);
}

byte readEEPROM(int addr) {
  // Make {address, OE = 1, WE = 0} available on pins of the EEPROM
  setIOPinsAs(INPUT);
  disableEEPROM();
  setShiftRegisters(addr, READ);
  enableEEPROM();

  // Read data into return variable 
  byte data = 0;
  for (int i = 0; i != 8; ++i) {
    data |= (digitalRead(IO_PINS[i]) << i);
  }

  disableEEPROM();
  return data;
}

void writeEEPROM(int addr, byte value) {

  // Make sure EEPROM is disabled while setting the outputs of the arduino
  disableEEPROM();

  // set output to value
  setOutput(value);

  // Make {address, OE = 1, WE = 0} available on pins of the EEPROM (OE and WE are active low)
  setShiftRegisters(addr, WRITE);

  // Pulse CE of the EEPROM to write the value into it
  enableEEPROM();
  delay(1); // CE needs to be held low at least 100ns
  disableEEPROM();
  delay(10); // It needs a rather big delay... if not, subsequent writes do not work for some reason.

  // set IO back to input to prevent accidental writes
  setIOPinsAs(INPUT);
}

void showPercentage(int p) {
      Serial.print("PROGRAMMER: ");
      Serial.print(p);
      Serial.println("%");
}

int showPercentage(float current, float total, int previous) {
    int newPercentage = current / total * 10;
    newPercentage *= 10;
    if (newPercentage > previous) {
      showPercentage(newPercentage);
      return newPercentage;
    }
    return previous;
}

void flash() {
  Serial.println("PROGRAMMER: Waiting for data transfer");
  while (!Serial.available()) {} // wait
  digitalWrite(STATUS_LED, LOW);
  Serial.println("PROGRAMMER: Receiving data ...");

  uint32_t dataLength = 0;
  Serial.readBytes(reinterpret_cast<byte*>(&dataLength), 4);
  dataLength = (dataLength <= MAX_ROM_SIZE) ? dataLength : MAX_ROM_SIZE;

  Serial.print("PROGRAMMER: writing ");
  Serial.print(dataLength);
  Serial.println(" bytes to EEPROM");

  int percentage = 0;
  for (int i = 0; i != min(dataLength, MAX_ROM_SIZE); ++i) {
    writeEEPROM(i, Serial.read());
    percentage = showPercentage(i, MAX_ROM_SIZE, percentage);
  }
  showPercentage(100);
}

void dump() {
  digitalWrite(STATUS_LED, LOW);
  static constexpr uint32_t dataLength = MAX_ROM_SIZE;
  Serial.write(reinterpret_cast<byte const*>(&dataLength), sizeof(dataLength));
  for (int i = 0; i != MAX_ROM_SIZE; ++i) {
    byte b = readEEPROM(i);
    Serial.write(b);
  }
}

void setup() {
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  pinMode(SHIFT_CLOCK, OUTPUT);
  setShiftRegisters(0, NONE); // make sure we're not accidentally writing, before setting the pinmode of CE to output
  pinMode(CE, OUTPUT);
  disableEEPROM();
  pinMode(STATUS_LED, OUTPUT);

  Serial.begin(9600);
  Serial.write(PROGRAMMER_READY); // let computer know we're ready
  digitalWrite(STATUS_LED, HIGH);

  while (true) { 
    byte c = Serial.read();
    if (c == DUMP_REQUEST) {
      dump(); 
      break;
    }
    else if (c == FLASH_REQUEST) {
      flash();
      break;
    }
  }

  Serial.write(PROGRAMMER_FINISHED);
  setIOPinsAs(INPUT);
  setShiftRegisters(0, NONE);
  disableEEPROM();
}

void loop() {
  // Blink to indicate done/idle
  digitalWrite(STATUS_LED, HIGH);
  delay(500);
  digitalWrite(STATUS_LED, LOW);
  delay(500);
}







