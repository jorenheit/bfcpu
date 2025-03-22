#include "keyboardbuffer.h"

void KeyboardBuffer::begin() {
  kb.begin();
}

void KeyboardBuffer::update() {
  while (kb.available()) {
    char const c = kb.read();
    ringBuf.put(c);
    if (settings.echoEnabled) echo(c);
  }
}

char KeyboardBuffer::get() {
  auto result = ringBuf.get();
  return result.good ? result.value : 0;
}

char KeyboardBuffer::peek() const {
  auto result = ringBuf.peek();
  return result.good ? result.value : 0;
}