#ifndef LIQUID_CRYSTAL_74HC595_H
#define LIQUID_CRYSTAL_74HC595_H

#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

#define LCD_MAX_ROWS 4

class LiquidCrystal_74HC595 : public Print {

  uint8_t const _cols;
  uint8_t const _rows;
  uint8_t const _row_offsets[LCD_MAX_ROWS];
  
  uint8_t _displayControl;
  uint8_t _displayMode;
  uint8_t _register;

public:
  LiquidCrystal_74HC595(uint8_t const cols, uint8_t const rows);
  
  void begin(uint8_t const charsize = LCD_5x8DOTS);
  void clear();
  void home();
  void noDisplay();
  void display();
  void noCursor();
  void cursor();
  void noBlink();
  void blink();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void leftToRight();
  void rightToLeft();
  void autoscroll();
  void noAutoscroll();
  void setCursor(uint8_t const col, uint8_t const row);
  void command(uint8_t const value);
  virtual size_t write(uint8_t const value);
  using Print::write;

private:
  enum LCDDataMode {
    COMMAND = 0,
    WRITE = 1
  };

  void send(uint8_t const value, LCDDataMode const mode);
  void pulseEnable();
  void write4bits(uint8_t const value);
  void transfer() const;
};

#endif
