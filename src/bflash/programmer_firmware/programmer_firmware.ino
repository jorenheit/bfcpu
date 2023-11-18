#define MAX_ROM_SIZE 1024
byte romBuffer[MAX_ROM_SIZE];

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

void enableEEPROM() {
  digitalWrite(CE, LOW);
}

void disableEEPROM() {
  digitalWrite(CE, LOW);
}

byte readEEPROM(int addr, bool disableAfterRead = true) {
  // Make {address, OE = 1, WE = 0} available on pins of the EEPROM
  setShiftRegisters(addr, READ);
  enableEEPROM();

  // Read data into return variable 
  byte data = 0;
  setIOPinsAs(INPUT);
  for (int pin = IO_0; pin <= IO_7; ++pin) {
    data |= (digitalRead(pin) << (pin - IO_0));
  }

  if (disableAfterRead) {
    disableEEPROM();
  }

  return data;
}

void readEEPROM(int start, int nBytes, byte *buf) {
  for (int i = 0; i != nBytes; ++i) {
    buf[i] = readEEPROM(start + i, /*disableAfterRead*/ false);
  }

  disableEEPROM();
}

void writeEEPROM(int addr, byte value) {

  // Make sure EEPROM is disabled while setting the outputs of the arduino
  digitalWrite(CE, HIGH);
  setIOPinsAs(OUTPUT);
  for (int pin = IO_0; pin <= IO_7; ++pin) {
    digitalWrite(pin, (value >> (pin - IO_0)) & 1);
  }

  // Make {address, OE = 0, WE = 1} available on pins of the EEPROM
  setShiftRegisters(addr, WRITE);

  // Pulse CE of the EEPROM to write the value into it
  enableEEPROM();
  delayMicroseconds(1); // CE needs to be held low at least 100ns
  disableEEPROM();

  // set IO back to input to prevent accidental writes
  setIOPinsAs(INPUT);
}

int showPercentage(float current, float total, int previous) {
    int newPercentage = current / total * 10;
    newPercentage *= 10;
    if (newPercentage > previous) {
      Serial.print("PROGRAMMER: ");
      Serial.print(newPercentage);
      Serial.println("%");
      return newPercentage;
    }
    return previous;
}

void flash() {
  Serial.println("PROGRAMMER: Waiting for data transfer");
  while (!Serial.available()) {} // wait
  Serial.println("PROGRAMMER: Receiving data ...");

  uint32_t dataLength = 0;
  Serial.readBytes(reinterpret_cast<byte*>(&dataLength), 4);
  dataLength = (dataLength <= MAX_ROM_SIZE) ? dataLength : MAX_ROM_SIZE;

  Serial.print("PROGRAMMER: writing ");
  Serial.print(dataLength);
  Serial.println(" bytes.");

  int percentage = 0;
  for (int i = 0; i != MAX_ROM_SIZE; ++i) {
    if (i < dataLength) {
      romBuffer[i] = Serial.read();
    }
    else {
      romBuffer[i] = 0;
    }
    percentage = showPercentage(i, MAX_ROM_SIZE, percentage);
  }

  Serial.println("PROGRAMMER: 100%");
  Serial.println("PROGRAMMER: Writing data to EEPROM");

  percentage = 0;
  for (int i = 0; i != MAX_ROM_SIZE; ++i) {
    writeEEPROM(i, romBuffer[i]);
    percentage = showPercentage(i, MAX_ROM_SIZE, percentage);
  }
  Serial.println("PROGRAMMER: 100%");
  Serial.println("PROGRAMMER: Done!");
  Serial.write(PROGRAMMER_FINISHED); // let computer know we're done
}

void dump() {
  readEEPROM(0, MAX_ROM_SIZE, romBuffer); 
  static constexpr uint32_t dataLength = MAX_ROM_SIZE;
  Serial.write(reinterpret_cast<byte const*>(&dataLength), sizeof(dataLength));
  Serial.write(romBuffer, MAX_ROM_SIZE);
}

void setup() {
  Serial.begin(9600);
  Serial.write(PROGRAMMER_READY); // let computer know we're ready

  while (true) { 
    byte c = Serial.read();
    if (c == DUMP_REQUEST) {
      dump(); 
      return;
    }
    else if (c == FLASH_REQUEST) {
      flash();
      return;
    }
  }
  
}

void loop() {

}







