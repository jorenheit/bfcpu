#include "PS2Keyboard.h"

namespace Impl {
  uint8_t CharBuffer = 0;
  uint8_t UTF8next = 0;
  RingBuffer<uint8_t, 256> ringBuf;

  void ps2interrupt()
  {
    static uint8_t bitCount = 0;
    static byte incomingData = 0;
    static unsigned long lastInterruptTime = 0;

    unsigned long currentTime = millis();
    if (currentTime - lastInterruptTime > KEYBOARD_TIMEOUT) {
      bitCount = 0;
      incomingData = 0;
    }
    lastInterruptTime = currentTime;

    if (bitCount > 0 && bitCount <= 8) {
      bool const dataBit = digitalRead<KEYBOARD_DATA_PIN>();
      incomingData |= (dataBit << (bitCount - 1));
    }

    if (++bitCount == 11) {
      ringBuf.put(incomingData);
      bitCount = 0;
      incomingData = 0;
    }
  }

  uint8_t get_scan_code()
  {
    auto const result = ringBuf.get();
    return result.ok ? result.value : 0;
  }

  #include "keymap.h"
  char get_iso8859_code()
  {
    enum: uint8_t {
      BREAK    = 0x01,
      MODIFIER = 0x02,
      SHIFT_L  = 0x04,
      SHIFT_R  = 0x08,
      ALTGR    = 0x10
    };

    static uint8_t state = 0;

    while (true) {
      uint8_t s = get_scan_code();
      if (!s) return 0;
      if (s == 0xF0) {
        state |= BREAK;
      } else if (s == 0xE0) {
        state |= MODIFIER;
      } else {
        if (state & BREAK) {
          if (s == 0x12) {
            state &= ~SHIFT_L;
          } else if (s == 0x59) {
            state &= ~SHIFT_R;
          } else if (s == 0x11 && (state & MODIFIER)) {
            state &= ~ALTGR;
          }
          state &= ~(BREAK | MODIFIER);
          continue;
        }
        if (s == 0x12) {
          state |= SHIFT_L;
          continue;
        } else if (s == 0x59) {
          state |= SHIFT_R;
          continue;
        } else if (s == 0x11 && (state & MODIFIER)) {
          state |= ALTGR;
        }
        char c = 0;
        if (state & MODIFIER) {
          switch (s) {
            case 0x70: c = PS2_INSERT;      break;
            case 0x6C: c = PS2_HOME;        break;
            case 0x7D: c = PS2_PAGEUP;      break;
            case 0x71: c = PS2_DELETE;      break;
            case 0x69: c = PS2_END;         break;
            case 0x7A: c = PS2_PAGEDOWN;    break;
            case 0x75: c = PS2_UPARROW;     break;
            case 0x6B: c = PS2_LEFTARROW;   break;
            case 0x72: c = PS2_DOWNARROW;   break;
            case 0x74: c = PS2_RIGHTARROW;  break;
            case 0x4A: c = '/';             break;
            case 0x5A: c = PS2_ENTER;       break;
            default: break;
          }
        } else if (state & (SHIFT_L | SHIFT_R)) {
          if (s < PS2_KEYMAP_SIZE)
            c = pgm_read_byte(shiftKeymap + s);
        } else {
          if (s < PS2_KEYMAP_SIZE)
            c = pgm_read_byte(noShiftKeymap + s);
        }
        state &= ~(BREAK | MODIFIER);
        if (c) return c;
      }
    }
  }
}

bool PS2Keyboard::available() {
  if (Impl::CharBuffer || Impl::UTF8next) return true;
  Impl::CharBuffer = Impl::get_iso8859_code();
  if (Impl::CharBuffer) return true;
  return false;
}

void PS2Keyboard::clear() {
  Impl::CharBuffer = 0;
  Impl::UTF8next = 0;
}

uint8_t PS2Keyboard::read() {
  uint8_t result = Impl::UTF8next;
  if (result) {
    Impl::UTF8next = 0;
  } else {
    result = Impl::CharBuffer;
    if (result) {
      Impl::CharBuffer = 0;
    } else {
      result = Impl::get_iso8859_code();
    }
    if (result >= 128) {
      Impl::UTF8next = (result & 0x3F) | 0x80;
      result = ((result >> 6) & 0x1F) | 0xC0;
    }
  }
  if (!result) return -1;
  return result;
}

void PS2Keyboard::begin() {
  pinMode(KEYBOARD_CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
  pinMode(KEYBOARD_DATA_PIN, INPUT_PULLUP);

  attachInterrupt(
    digitalPinToInterrupt(KEYBOARD_CLOCK_INTERRUPT_PIN), 
    Impl::ps2interrupt, 
    FALLING
  );
}