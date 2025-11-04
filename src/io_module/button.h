#pragma once
#include "settings.h"
#include "common.h"

/*
USAGE:

Create buttons using the Button::create function, passing the pin-number as a template parameter:
auto button1 = Button::create<Pin1>();
auto button2 = Button::create<Pin2>();

These buttons will register as pressed when a high logic level is detected at the specified pins. 
To create a button that is active low, supply a seconde template argument:

auto button3 = Button::create<Pin3, LOW>();

It's also possible to create a button button from existing buttons. This button will act as any 
other button and register as being pressed when both its child-buttons are being pressed. 

auto button4 = ButtonPair::create(button1, button2);

Possible states of a button are:
* Pressed
* Released
* JustPressed
* JustReleased
* Hold

Any button will provide the following interface:

void begin();          -> call this after construction to properly setup the button
ButtonState update();  -> updates its state and returns this.
ButtonState state();   -> returns the current state without updating.
bool isDown();         -> true if button-state is Pressed, JustPressed or Hold
bool isUp();           -> true button-state is Released or JustReleased
bool isJustPressed();  -> true if button-state is JustPressed
bool isJustReleased(); -> true if button-state is JustReleased
bool isHold();         -> true if button-state is Hold
*/

enum class ButtonState {
  Pressed,
  Released,
  JustPressed,
  JustReleased,
  Hold
};

template <typename Derived>
class ButtonBase_ {
  ButtonState currentState = ButtonState::Released;
  unsigned long lastStateChangeTime = 0;

public:
  inline void begin() {
    static_cast<Derived*>(this)->begin_();
  }

  ButtonState update() {
    if (currentState == ButtonState::JustPressed) return (currentState = ButtonState::Pressed);
    else if (currentState == ButtonState::JustReleased) return (currentState = ButtonState::Released);

    unsigned long currentTime = millis();
    if (currentTime - lastStateChangeTime < BUTTON_DEBOUNCE_DELAY)
      return currentState;

    bool const buttonDown = static_cast<Derived*>(this)->active();
    if (!isDown() && buttonDown) {
      lastStateChangeTime = currentTime;
      return (currentState = ButtonState::JustPressed);
    }
    else if (isDown() && !buttonDown) {
      lastStateChangeTime = currentTime;
      return (currentState = ButtonState::JustReleased);
    }
    else if (isDown() && buttonDown && (currentTime - lastStateChangeTime > BUTTON_HOLD_TIME)) {
      return (currentState = ButtonState::Hold);
    }
    else return currentState;
  }

  inline ButtonState state() const {
    return currentState;
  }

  inline bool isDown() const {
    return currentState == ButtonState::JustPressed || currentState == ButtonState::Pressed || currentState == ButtonState::Hold;
  }

  inline bool isJustPressed() const {
    return currentState == ButtonState::JustPressed;
  }

  inline bool isJustReleased() const {
    return currentState == ButtonState::JustReleased;
  }

  inline bool isHold() const {
    return currentState == ButtonState::Hold;
  }
};

namespace Button {
  template <int Pin, bool ActiveLevel = HIGH>
  class Button_;

  template <int Pin, bool ActiveLevel = HIGH>
  inline Button_<Pin, ActiveLevel> create() {
    return {};
  }

  template <int Pin, bool ActiveLevel>
  class Button_: public ButtonBase_<Button_<Pin, ActiveLevel>> {
    friend class ButtonBase_<Button_<Pin, ActiveLevel>>;
    friend Button_ create<Pin, ActiveLevel>();

    Button_() = default;
    inline void begin_() {
      pinMode(Pin, INPUT);
    }

    inline bool active() const {
      return digitalRead<Pin>() == ActiveLevel;
    }
  };
}

namespace ButtonPair {
  template <typename Button1, typename Button2>
  class ButtonPair_;

  template <typename Button1, typename Button2>
  inline ButtonPair_<Button1, Button2> create(Button1 &b1, Button2 &b2) {
    return ButtonPair_<Button1, Button2>{b1, b2};
  }

  template <typename Button1, typename Button2>
  class ButtonPair_: public ButtonBase_<ButtonPair_<Button1, Button2>> {
    friend class ButtonBase_<ButtonPair_<Button1, Button2>>;
    friend ButtonPair_ create<Button1, Button2>(Button1 &, Button2 &);

    Button1 &button1;
    Button2 &button2;

    ButtonPair_(Button1 &b1, Button2 &b2):
      button1(b1),
      button2(b2)
    {}

    inline void begin_() {
      button1.begin();
      button2.begin();
    }

    inline bool active() const {
      return button1.isDown() && button2.isDown();
    }
  };
}
