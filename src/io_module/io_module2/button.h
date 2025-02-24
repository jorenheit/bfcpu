#pragma once
#include "settings.h"
#include "common.h"

enum class ButtonState {
  Low,
  High,
  Rising,
  Falling
};

class ButtonBase {
private:
  ButtonState currentState = ButtonState::Low;
  unsigned long lastStateChangeTime = 0;

protected:
  virtual bool read() = 0;

public:
  ButtonState state() {
    if (currentState == ButtonState::Rising) return (currentState = ButtonState::High);
    else if (currentState == ButtonState::Falling) return currentState = ButtonState::Low;

    unsigned long currentTime = millis();
    if (currentTime - lastStateChangeTime < BUTTON_DEBOUNCE_DELAY)
      return currentState;

    bool const pinState = this->read();
    if (currentState == ButtonState::Low && pinState) {
      currentState = ButtonState::Rising;
      lastStateChangeTime = currentTime;
    }
    else if (currentState == ButtonState::High && !pinState) {
      currentState = ButtonState::Falling;
      lastStateChangeTime = currentTime;
    }

    return currentState;
  }
};

template <int Pin>
class Button: public ButtonBase {
public:
  void begin() {
    pinMode(Pin, INPUT);
  }

private:
  virtual bool read() {
    return digitalRead<Pin>();
  }
};

class ButtonPair: public ButtonBase {
  ButtonBase *button1;
  ButtonBase *button2;

public:
  ButtonPair(ButtonBase &b1, ButtonBase &b2):
    button1(&b1),
    button2(&b2)
  {}

private:
  virtual bool read() {
    ButtonState const s1 = button1->state();
    ButtonState const s2 = button2->state();

    bool const b1High = (s1 == ButtonState::Rising) || (s1 == ButtonState::High);
    bool const b2High = (s2 == ButtonState::Rising) || (s2 == ButtonState::High);
    return b1High && b2High;
  }
};