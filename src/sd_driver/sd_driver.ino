#include <SD.h>

/*

  Input to the module: 
  instruction pointer -> 16 bit input -> need 2x 8 bit multiplexer (74LS151)
  Connect the select lines (3x) together and run the outputs of the 151 to 2 different input pins to 
  read the 2 bytes in parallel into memory. Something like:
*/

uint16_t getAddress() {
    uint16_t lowByte = 0;
    uint16_t highByte = 0;
    for (int bit = 0; bit != 8; ++bit) {
        digitalWrite(SELECT0, bit & 0b001);
        digitalWrite(SELECT1, bit & 0b010);
        digitalWrite(SELECT2, bit & 0b100);

        lowByte |= (digitalRead(LOW_BYTE_BIT) << bit);
        highByte |= (digitalRead(HIGH_BYTE_BIT) << bit);
    }

    return (highByte << 8) | lowByte;
}

/*
  Number of pins needed: 3 for bit-select, 2 inputs -> 5
  This leaves enough pins for parallel output (4 bits) of the result without using shift registers.
  This allows us to fetch an address, which we use as an index into the file.

  uint16_t addr = getAddress();
  file.seek(addr);
  byte value = SD.peek();
  --> output this value to output pins

  Should this happen in an interrupt? I guess so -> 1 additional pin needed

  Having a 2nd LCD for file selection would be nice, but it would
  require a bunch of additional pins (for controlling the LCD +
  buttons). For now, just select the file through the serial monitor of the arduino
  IDE. Maybe have a switch to indicate if the module should wait for
  commands over serial or pick try to load a hardcoded filename.

*/

enum Pins {
    // To select-pins of the muxers
    BIT_SELECT0 = 2,
    BIT_SELECT1 = 3,
    BIT_SELECT2 = 4,

    // From muxer outputs 
    LOW_BYTE_BIT = 5,
    HIGH_BYTE_BIT = 6,

    // From a switch
    FILE_SELECT_MODE = 7,
    SD_CS = 8,

    // Data out (4 bits per command)
    D0 = 9,
    D1 = 10,
    D2 = 11,
    D3 = 12
};

enum FileSelectMode {
    USE_DEFAULT_FILENAME = LOW,
    SELECT_VIA_SERIAL = HIGH
};


String const defaultFilename("program.bin");
bool ready = false;

File selectFile(){
    // Try to open the default file 
    if (digitalRead(FILE_SELECT_MODE) == USE_DEFAULT_FILENAME && SD.exists(defaultFilename)) {
        File file = SD.open(defaultFilename, FILE_READ);
        if (file) {
            return file;
        }
        else {
            Serial.print("ERROR: Coult not open file ");
            Serial.print(defaultFilename);
            Serial.println(", switching to interactive mode.");
        }
    }

    // Enter interactive prompt ==> check root directory for files
    File dir = SD.open("/");
    String fileListing;
    int fileCount = 0;
    while (true) {
        File file = dir.openNextFile();
        if (file) {
            if (file.isDirectory())
                continue; // ignore directories, only list files in root

            fileListing += String(file.name()) + '\n';
            ++fileCount;
        }
        else
            break;
    }
    if (fileCount == 0) {
        fileListing = "-";  
    }

    // Prompt user for input
    while (true) {
        Serial.println("Files available: ");
        Serial.println(fileListing);
        Serial.print("Total: ");
        Serial.print(fileCount);
        Serial.println(" files available.");
        if (fileCount > 0) {
            Serial.println("Please enter file to load.");
        }
        else {
            Serial.println("Please copy files onto the SD card and reset... Aborting");
            return File{};
        }

        while (!Serial.available()) {
            delay(100);
        }
        
        String filename = Serial.readString();
        filename.trim();
        File file = SD.open(filename);
        if (file) {
            Serial.print("Successfully opened ");
            Serial.println(filename);
            return file;
        }
        else {
            Serial.print("ERROR: Could not open file ");
            Serial.println(filename);
        }
    }
}


void setup() {
    pinMode(BIT_SELECT0, OUTPUT);
    pinMode(BIT_SELECT1, OUTPUT);
    pinMode(BIT_SELECT2, OUTPUT);

    pinMode(LOW_BYTE_BIT, INPUT);
    pinMode(HIGH_BYTE_BIT, INPUT);

    pinMode(FILE_SELECT_MODE, INPUT);

    for (int pin = D0; pin != D3; ++pin)
        pinMode(pin, OUTPUT);
    
    Serial.begin(9600);
    while (!Serial) {} // wait for serial to get ready

    if (!SD.begin(SD_CS)) {
        Serial.println("SD card initialization failed");
        return;
    }

    File file = selectFile();
}

void loop() {
    if (ready) {
        Serial.println("Ready");
    }
    else {
        Serial.println("Not ready");
        delay(1000);
    }
}
