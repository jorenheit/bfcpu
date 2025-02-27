#include "keyboardbuffer.h"

void KeyboardBuffer::begin() {
  kb.begin();
}

void KeyboardBuffer::update() {
  while (kb.available()) {
    ringBuf.put(kb.read());
  }
}

char KeyboardBuffer::get() {
  auto result = ringBuf.get();
  return result.ok ? result.value : 0;
}

char KeyboardBuffer::peek() const {
  auto result = ringBuf.peek();
  return result.ok ? result.value : 0;
}