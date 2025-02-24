#pragma once
#include "settings.h"
#include "common.h"


template <int Pin>
class Button {
public:
  bool pressed() const {
    static unsigned long lastPressedTime = millis();
    unsigned long const currentTime = millis();
    if (currentTime - lastPressedTime < BUTTON_DEBOUNCE_DELAY)
      return false;

    if (digitalRead<Pin>()) {
      lastPressedTime = currentTime;
      return true;
    }
    return false;
  }
};