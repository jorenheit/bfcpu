#pragma once
#include "settings.h"
#include "common.h"

enum class ButtonState {
  Low,
  High,
  Rising,
  Falling,
  Hold
};

class ButtonBase {
private:
  ButtonState currentState = ButtonState::Low;
  unsigned long lastStateChangeTime = 0;

protected:
  virtual bool isPressed() = 0;

public:
  ButtonState state() {
    if (currentState == ButtonState::Rising) return (currentState = ButtonState::High);
    else if (currentState == ButtonState::Falling) return (currentState = ButtonState::Low);

    unsigned long currentTime = millis();
    if (currentTime - lastStateChangeTime < BUTTON_DEBOUNCE_DELAY)
      return currentState;

    bool const pressed = this->isPressed();
    if (currentState == ButtonState::Low && pressed) {
      currentState = ButtonState::Rising;
      lastStateChangeTime = currentTime;
    }
    else if (currentState == ButtonState::High && !pressed) {
      currentState = ButtonState::Falling;
      lastStateChangeTime = currentTime;
    }
    else if (currentState == ButtonState::High && pressed){
      if (currentTime - lastStateChangeTime > BUTTON_HOLD_TIME) {
        // currentState remains High, but return Hold to indicate it's been high a while
        return ButtonState::Hold;
      }
    }
    return currentState;
  }

  ButtonState state() const {
    return currentState;
  }
};

template <int Pin, bool PressedLevel = HIGH>
class Button: public ButtonBase {
public:
  void begin() {
    pinMode(Pin, INPUT);
  }

private:
  virtual bool isPressed() {
    return digitalRead<Pin>() == PressedLevel;
  }
};

struct ChangeState {
  using ButtonBaseRef = ButtonBase &;
};

struct PeekState {
  using ButtonBaseRef = ButtonBase const &;
};

template <typename StatePolicy>
class ButtonPair: public ButtonBase {
  using ButtonBaseRef = typename StatePolicy::ButtonBaseRef;
  
  ButtonBaseRef button1;
  ButtonBaseRef button2;

public:
  ButtonPair(ButtonBaseRef b1, ButtonBaseRef b2):
    button1(b1),
    button2(b2)
  {}

private:
  virtual bool isPressed() {
    ButtonState const s1 = button1.state();
    ButtonState const s2 = button2.state();

    bool const b1High = (s1 == ButtonState::Rising) || (s1 == ButtonState::High);
    bool const b2High = (s2 == ButtonState::Rising) || (s2 == ButtonState::High);
    return b1High && b2High;
  }
};