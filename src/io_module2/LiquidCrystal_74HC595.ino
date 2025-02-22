#if USE_FAST_LIQUIDCRYSTAL_LIBRARY
#include "LiquidCrystal_74HC595.h"

LiquidCrystal_74HC595::LiquidCrystal_74HC595(uint8_t const cols, uint8_t const rows):
  _cols(cols),
  _rows(min(rows, LCD_MAX_ROWS)),
  _row_offsets{0x00, 0x40, (uint8_t)(0x00 + _cols), (uint8_t)(0x40 + _cols)}
{}

void LiquidCrystal_74HC595::begin(uint8_t const charsize) {
  pinMode(DS, OUTPUT);
  pinMode(SH_CP, OUTPUT);
  pinMode(ST_CP, OUTPUT);
  
  _displayControl = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
  _displayMode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

  delayMicroseconds(50000);
  write4bits(0x03);
  delayMicroseconds(4500);
  write4bits(0x03);
  delayMicroseconds(4500);
  write4bits(0x03);
  delayMicroseconds(150);
  write4bits(0x02);

  if (_rows > 1) {
    command(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);
  } else if (_rows == 1 && charsize != 0) {
    command(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_1LINE | LCD_5x10DOTS);
  } else {
    command(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS);
  }

  command(LCD_DISPLAYCONTROL | _displayControl);
  clear();
  command(LCD_ENTRYMODESET | _displayMode);
}

void LiquidCrystal_74HC595::clear() {
  command(LCD_CLEARDISPLAY);
  delayMicroseconds(2000);
}

void LiquidCrystal_74HC595::home() {
  command(LCD_RETURNHOME);
  delayMicroseconds(2000);
}

void LiquidCrystal_74HC595::noDisplay() {
  _displayControl &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displayControl);
}

void LiquidCrystal_74HC595::display() {
  _displayControl |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displayControl);
}

void LiquidCrystal_74HC595::noCursor() {
  _displayControl &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displayControl);
}

void LiquidCrystal_74HC595::cursor() {
  _displayControl |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displayControl);
}

void LiquidCrystal_74HC595::noBlink() {
  _displayControl &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displayControl);
}

void LiquidCrystal_74HC595::blink() {
  _displayControl |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displayControl);
}

void LiquidCrystal_74HC595::scrollDisplayLeft() {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void LiquidCrystal_74HC595::scrollDisplayRight() {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void LiquidCrystal_74HC595::leftToRight() {
  _displayMode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displayMode);
}

void LiquidCrystal_74HC595::rightToLeft() {
  _displayMode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displayMode);
}

void LiquidCrystal_74HC595::autoscroll() {
  _displayMode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displayMode);
}

void LiquidCrystal_74HC595::noAutoscroll() {
  _displayMode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displayMode);
}

void LiquidCrystal_74HC595::setCursor(uint8_t const col, uint8_t const row) {
  command(LCD_SETDDRAMADDR | (col + _row_offsets[min(row, _rows - 1)]));
}

inline void LiquidCrystal_74HC595::command(uint8_t const value) {
  send(value, 0);
}

inline size_t LiquidCrystal_74HC595::write(uint8_t const value) {
  send(value, 1);
  return 1;
}

void LiquidCrystal_74HC595::send(uint8_t const value, uint8_t const mode) {
  bitWrite(_register, RS_595, mode);
  transfer();
  write4bits(value >> 4);
  write4bits(value);
}

void LiquidCrystal_74HC595::pulseEnable() {
  bitWrite(_register, E_595, 0); 
  transfer();
  delayMicroseconds(1);
  bitWrite(_register, E_595, 1);
  transfer();
  delayMicroseconds(1);
  bitWrite(_register, E_595, 0);
  transfer();
  delayMicroseconds(LCD_COMMAND_EXECUTION_MICROS);
}

void LiquidCrystal_74HC595::write4bits(uint8_t const value) {
  bitWrite(_register, D0_595, (value >> 0) & 1);
  bitWrite(_register, D1_595, (value >> 1) & 1);
  bitWrite(_register, D2_595, (value >> 2) & 1);
  bitWrite(_register, D3_595, (value >> 3) & 1);
  transfer();
  pulseEnable();
}

void LiquidCrystal_74HC595::transfer() const {
  digitalWrite<ST_CP, LOW>();
  shiftOut<DS, SH_CP, MSBFIRST>(_register);
  digitalWrite<ST_CP, HIGH>();
}

#endif //USE_FAST_LIQUIDCRYSTAL_LIBRARY